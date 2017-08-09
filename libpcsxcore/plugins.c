/***************************************************************************
 *   Copyright (C) 2007 Ryan Schultz, PCSX-df Team, PCSX team              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02111-1307 USA.           *
 ***************************************************************************/

/*
* Plugin library callback/access functions.
*/

#include "plugins.h"
#include "cdriso.h"

static char IsoFile[MAXPATHLEN] = "";
static s64 cdOpenCaseTime = 0;

CDRinit               CDR_init;
CDRshutdown           CDR_shutdown;
CDRopen               CDR_open;
CDRclose              CDR_close; 
CDRtest               CDR_test;
CDRgetTN              CDR_getTN;
CDRgetTD              CDR_getTD;
CDRreadTrack          CDR_readTrack;
CDRgetBuffer          CDR_getBuffer;
CDRplay               CDR_play;
CDRstop               CDR_stop;
CDRgetStatus          CDR_getStatus;
CDRgetDriveLetter     CDR_getDriveLetter;
CDRgetBufferSub       CDR_getBufferSub;
CDRconfigure          CDR_configure;
CDRabout              CDR_about;
CDRsetfilename        CDR_setfilename;
CDRreadCDDA           CDR_readCDDA;
CDRgetTE              CDR_getTE;


#ifdef ENABLE_SIO1API

SIO1init              SIO1_init;
SIO1shutdown          SIO1_shutdown;
SIO1open              SIO1_open;
SIO1close             SIO1_close; 
SIO1test              SIO1_test;
SIO1configure         SIO1_configure;
SIO1about             SIO1_about;
SIO1pause             SIO1_pause;
SIO1resume            SIO1_resume;
SIO1keypressed        SIO1_keypressed;
SIO1writeData8        SIO1_writeData8;
SIO1writeData16       SIO1_writeData16;
SIO1writeData32       SIO1_writeData32;
SIO1writeStat16       SIO1_writeStat16;
SIO1writeStat32       SIO1_writeStat32;
SIO1writeMode16       SIO1_writeMode16;
SIO1writeMode32       SIO1_writeMode32;
SIO1writeCtrl16       SIO1_writeCtrl16;
SIO1writeCtrl32       SIO1_writeCtrl32;
SIO1writeBaud16       SIO1_writeBaud16;
SIO1writeBaud32       SIO1_writeBaud32;
SIO1readData8         SIO1_readData8;
SIO1readData16        SIO1_readData16;
SIO1readData32        SIO1_readData32;
SIO1readStat16        SIO1_readStat16;
SIO1readStat32        SIO1_readStat32;
SIO1readMode16        SIO1_readMode16;
SIO1readMode32        SIO1_readMode32;
SIO1readCtrl16        SIO1_readCtrl16;
SIO1readCtrl32        SIO1_readCtrl32;
SIO1readBaud16        SIO1_readBaud16;
SIO1readBaud32        SIO1_readBaud32;
SIO1registerCallback  SIO1_registerCallback;

#endif

static const char *err;

#define CheckErr(func) { \
	err = SysLibError(); \
	if (err != NULL) { SysMessage(_("Error loading %s: %s"), func, err); return -1; } \
}

void CALLBACK GPU__displayText(char *pText) {
	SysPrintf("%s\n", pText);
}

long CALLBACK GPU__configure(void) { return 0; }
long CALLBACK GPU__test(void) { return 0; }
void CALLBACK GPU__about(void) {}
void CALLBACK GPU__makeSnapshot(void) {}
void CALLBACK GPU__keypressed(int key) {}
long CALLBACK GPU__getScreenPic(unsigned char *pMem) { return -1; }
long CALLBACK GPU__showScreenPic(unsigned char *pMem) { return -1; }
void CALLBACK GPUclearDynarec(void (CALLBACK *callback)(void)) {}
void CALLBACK GPUvBlank(int val) {}

void *hCDRDriver = NULL;

long CALLBACK CDR__play(unsigned char *sector) { return 0; }
long CALLBACK CDR__stop(void) { return 0; }

long CALLBACK CDR__getStatus(struct CdrStat *stat) {
	if (cdOpenCaseTime < 0 || cdOpenCaseTime > (s64)time(NULL))
		stat->Status = 0x10;
	else
		stat->Status = 0;

	return 0;
}

char* CALLBACK CDR__getDriveLetter(void) { return NULL; }
long CALLBACK CDR__configure(void) { return 0; }
long CALLBACK CDR__test(void) { return 0; }
void CALLBACK CDR__about(void) {}
long CALLBACK CDR__setfilename(char*filename) { return 0; }


long CALLBACK SPU__configure(void) { return 0; }
void CALLBACK SPU__about(void) {}
long CALLBACK SPU__test(void) { return 0; }

static unsigned char buf[256];
unsigned char stdpar[10] = { 0x00, 0x41, 0x5a, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
unsigned char mousepar[8] = { 0x00, 0x12, 0x5a, 0xff, 0xff, 0xff, 0xff };
unsigned char analogpar[9] = { 0x00, 0xff, 0x5a, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

static int bufcount, bufc;

PadDataS padd1, padd2;

unsigned char _PADstartPoll(PadDataS *pad) {
    bufc = 0;

    switch (pad->controllerType) {
        case PSE_PAD_TYPE_MOUSE:
            mousepar[3] = pad->buttonStatus & 0xff;
            mousepar[4] = pad->buttonStatus >> 8;
            mousepar[5] = pad->moveX;
            mousepar[6] = pad->moveY;

            memcpy(buf, mousepar, 7);
            bufcount = 6;
            break;
        case PSE_PAD_TYPE_NEGCON: // npc101/npc104(slph00001/slph00069)
            analogpar[1] = 0x23;
            analogpar[3] = pad->buttonStatus & 0xff;
            analogpar[4] = pad->buttonStatus >> 8;
            analogpar[5] = pad->rightJoyX;
            analogpar[6] = pad->rightJoyY;
            analogpar[7] = pad->leftJoyX;
            analogpar[8] = pad->leftJoyY;

            memcpy(buf, analogpar, 9);
            bufcount = 8;
            break;
        case PSE_PAD_TYPE_ANALOGPAD: // scph1150
            analogpar[1] = 0x73;
            analogpar[3] = pad->buttonStatus & 0xff;
            analogpar[4] = pad->buttonStatus >> 8;
            analogpar[5] = pad->rightJoyX;
            analogpar[6] = pad->rightJoyY;
            analogpar[7] = pad->leftJoyX;
            analogpar[8] = pad->leftJoyY;

            memcpy(buf, analogpar, 9);
            bufcount = 8;
            break;
        case PSE_PAD_TYPE_ANALOGJOY: // scph1110
            analogpar[1] = 0x53;
            analogpar[3] = pad->buttonStatus & 0xff;
            analogpar[4] = pad->buttonStatus >> 8;
            analogpar[5] = pad->rightJoyX;
            analogpar[6] = pad->rightJoyY;
            analogpar[7] = pad->leftJoyX;
            analogpar[8] = pad->leftJoyY;

            memcpy(buf, analogpar, 9);
            bufcount = 8;
            break;
        case PSE_PAD_TYPE_STANDARD:
        default:
            stdpar[3] = pad->buttonStatus & 0xff;
            stdpar[4] = pad->buttonStatus >> 8;

            memcpy(buf, stdpar, 5);
            bufcount = 4;
    }

    return buf[bufc++];
}

unsigned char _PADpoll(unsigned char value) {
    if (bufc > bufcount) return 0;
    return buf[bufc++];
}

unsigned char CALLBACK PAD1__startPoll(int pad) {
    PadDataS padd;

    PAD1_readPort1(&padd);

    return _PADstartPoll(&padd);
}

unsigned char CALLBACK PAD1__poll(unsigned char value) {
    return _PADpoll(value);
}

long CALLBACK PAD1__configure(void) { return 0; }
void CALLBACK PAD1__about(void) {}
long CALLBACK PAD1__test(void) { return 0; }
long CALLBACK PAD1__query(void) { return 3; }
long CALLBACK PAD1__keypressed() { return 0; }


long CALLBACK PAD2__configure(void) { return 0; }
void CALLBACK PAD2__about(void) {}
long CALLBACK PAD2__test(void) { return 0; }
long CALLBACK PAD2__query(void) { return PSE_PAD_USE_PORT1 | PSE_PAD_USE_PORT2; }
long CALLBACK PAD2__keypressed() { return 0; }


void CALLBACK clearDynarec(void) {
	psxCpu->Reset();
}

int LoadPlugins() {
	int ret;
	char Plugin[MAXPATHLEN];
	printf("LoadPlugins()\n");
	cdrIsoInit();
	printf("cdrisoInit done\n");

	Config.UseNet = FALSE;

	ret = CDR_init();
	if (ret < 0) { SysMessage (_("Error initializing CD-ROM plugin: %d"), ret); return -1; }
    else printf("cdr_init done\n");
	ret = GPU_init();
	if (ret < 0) { SysMessage (_("Error initializing GPU plugin: %d"), ret); return -1; }
    else printf("gpu_init done\n");
	ret = SPU_init();
	if (ret < 0) { SysMessage (_("Error initializing SPU plugin: %d"), ret); return -1; }
    else printf("spu_init done\n");
	ret = PAD1_init(1);
	if (ret < 0) { SysMessage (_("Error initializing Controller 1 plugin: %d"), ret); return -1; }
	ret = PAD2_init(2);
	if (ret < 0) { SysMessage (_("Error initializing Controller 2 plugin: %d"), ret); return -1; }

	SysPrintf(_("Plugins loaded.\n"));
	return 0;
}

void SetIsoFile(const char *filename) {
	if (filename == NULL) {
		IsoFile[0] = '\0';
		return;
	}
	strncpy(IsoFile, filename, MAXPATHLEN);
}

const char *GetIsoFile(void) {
	return IsoFile;
}

boolean UsingIso(void) {
	return (IsoFile[0] != '\0');
}

void SetCdOpenCaseTime(s64 time) {
	cdOpenCaseTime = time;
}
