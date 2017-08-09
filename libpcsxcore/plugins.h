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

#ifndef __PLUGINS_H__
#define __PLUGINS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "psxcommon.h"

//#define ENABLE_SIO1API 1

typedef void* HWND;
#define CALLBACK

long GPUopen(unsigned long *, char *, char *);
long SPUopen(void);
long PADopen(unsigned long *);
//typedef long (*NETopen)(unsigned long *);
//typedef long (*SIO1open)(unsigned long *);

#include "spu.h"

#include "psemu_plugin_defs.h"
#include "decode_xa.h"

int LoadPlugins();
void ReleasePlugins();
int OpenPlugins();
void ClosePlugins();

typedef unsigned long (CALLBACK* PSEgetLibType)(void);
typedef unsigned long (CALLBACK* PSEgetLibVersion)(void);
typedef char *(CALLBACK* PSEgetLibName)(void);

// GPU Functions
extern long GPUinit(void);
extern long GPUshutdown(void);
extern long GPUclose(void);
extern void GPUwriteStatus(uint32_t);
extern void GPUwriteData(uint32_t);
extern void GPUwriteDataMem(uint32_t *, int);
extern uint32_t GPUreadStatus(void);
extern uint32_t GPUreadData(void);
extern void GPUreadDataMem(uint32_t *, int);
extern long GPUdmaChain(uint32_t *,uint32_t);
extern void GPUupdateLace(void);
extern long GPUconfigure(void);
extern long GPUtest(void);
extern void GPUabout(void);
extern void GPUmakeSnapshot(void);
extern void GPUkeypressed(int);
extern void GPUdisplayText(char *);
typedef struct {
	uint32_t ulFreezeVersion;
	uint32_t ulStatus;
	uint32_t ulControl[256];
	unsigned char psxVRam[1024*512*2];
} GPUFreeze_t;
extern long GPUfreeze(uint32_t, GPUFreeze_t *);
extern long GPUgetScreenPic(unsigned char *);
extern long GPUshowScreenPic(unsigned char *);
extern void GPUclearDynarec(void (CALLBACK *callback)(void));
extern void GPUvBlank(int);

// GPU function pointers

#define GPU_updateLace    GPUupdateLace
#define GPU_init          GPUinit
#define GPU_shutdown      GPUshutdown
#define GPU_configure     GPUconfigure
#define GPU_test          GPUtest
#define GPU_about         GPUabout
#define GPU_open          GPUopen
#define GPU_close         GPUclose
#define GPU_readStatus    GPUreadStatus
#define GPU_readData      GPUreadData
#define GPU_readDataMem   GPUreadDataMem
#define GPU_writeStatus   GPUwriteStatus
#define GPU_writeData     GPUwriteData
#define GPU_writeDataMem  GPUwriteDataMem
#define GPU_dmaChain      GPUdmaChain
#define GPU_keypressed    GPUkeypressed
#define GPU_displayText   GPUdisplayText
#define GPU_makeSnapshot  GPUmakeSnapshot
#define GPU_freeze        GPUfreeze
#define GPU_getScreenPic  GPUgetScreenPic
#define GPU_showScreenPic GPUshowScreenPic
#define GPU_clearDynarec  GPUclearDynarec
#define GPU_vBlank        GPUvBlank


// CD-ROM Functions
typedef long (CALLBACK* CDRinit)(void);
typedef long (CALLBACK* CDRshutdown)(void);
typedef long (CALLBACK* CDRopen)(void);
typedef long (CALLBACK* CDRclose)(void);
typedef long (CALLBACK* CDRgetTN)(unsigned char *);
typedef long (CALLBACK* CDRgetTD)(unsigned char, unsigned char *);
typedef long (CALLBACK* CDRreadTrack)(unsigned char *);
typedef unsigned char* (CALLBACK* CDRgetBuffer)(void);
typedef unsigned char* (CALLBACK* CDRgetBufferSub)(void);
typedef long (CALLBACK* CDRconfigure)(void);
typedef long (CALLBACK* CDRtest)(void);
typedef void (CALLBACK* CDRabout)(void);
typedef long (CALLBACK* CDRplay)(unsigned char *);
typedef long (CALLBACK* CDRstop)(void);
typedef long (CALLBACK* CDRsetfilename)(char *);
struct CdrStat {
	uint32_t Type;
	uint32_t Status;
	unsigned char Time[3];
};
typedef long (CALLBACK* CDRgetStatus)(struct CdrStat *);
typedef char* (CALLBACK* CDRgetDriveLetter)(void);
struct SubQ {
	char res0[12];
	unsigned char ControlAndADR;
	unsigned char TrackNumber;
	unsigned char IndexNumber;
	unsigned char TrackRelativeAddress[3];
	unsigned char Filler;
	unsigned char AbsoluteAddress[3];
	unsigned char CRC[2];
	char res1[72];
};
typedef long (CALLBACK* CDRreadCDDA)(unsigned char, unsigned char, unsigned char, unsigned char *);
typedef long (CALLBACK* CDRgetTE)(unsigned char, unsigned char *, unsigned char *, unsigned char *);

// CD-ROM function pointers
extern CDRinit               CDR_init;
extern CDRshutdown           CDR_shutdown;
extern CDRopen               CDR_open;
extern CDRclose              CDR_close; 
extern CDRtest               CDR_test;
extern CDRgetTN              CDR_getTN;
extern CDRgetTD              CDR_getTD;
extern CDRreadTrack          CDR_readTrack;
extern CDRgetBuffer          CDR_getBuffer;
extern CDRgetBufferSub       CDR_getBufferSub;
extern CDRplay               CDR_play;
extern CDRstop               CDR_stop;
extern CDRgetStatus          CDR_getStatus;
extern CDRgetDriveLetter     CDR_getDriveLetter;
extern CDRconfigure          CDR_configure;
extern CDRabout              CDR_about;
extern CDRsetfilename        CDR_setfilename;
extern CDRreadCDDA           CDR_readCDDA;
extern CDRgetTE              CDR_getTE;

// SPU Functions
long (SPUinit)(void);				
long (SPUshutdown)(void);	
long (SPUclose)(void);			
void (SPUplaySample)(unsigned char);		
void (SPUwriteRegister)(unsigned long, unsigned short);
unsigned short (SPUreadRegister)(unsigned long);
void (SPUwriteDMA)(unsigned short);
unsigned short (SPUreadDMA)(void);
void SPUwriteDMAMem(unsigned short *, int);
void (SPUreadDMAMem)(unsigned short *, int);
void (SPUplayADPCMchannel)(xa_decode_t *);
void (SPUregisterCallback)(void (CALLBACK *callback)(void));
long (SPUconfigure)(void);
long (SPUtest)(void);
void (SPUabout)(void);
typedef struct {
	unsigned char PluginName[8];
	uint32_t PluginVersion;
	uint32_t Size;
	unsigned char SPUPorts[0x200];
	unsigned char SPURam[0x80000];
	xa_decode_t xa;
	unsigned char *SPUInfo;
} SPUFreeze_t;
long (SPUfreeze)(uint32_t, SPUFreeze_t *);
void (SPUasync)(uint32_t);
void (SPUplayCDDAchannel)(short *, int);

// SPU function pointers
#define SPU_configure        SPUconfigure
#define SPU_about            SPUabout
#define SPU_init             SPUinit
#define SPU_shutdown         SPUshutdown
#define SPU_test             SPUtest
#define SPU_open             SPUopen
#define SPU_close            SPUclose
#define SPU_playSample       SPUplaySample
#define SPU_writeRegister    SPUwriteRegister
#define SPU_readRegister     SPUreadRegister
#define SPU_writeDMA         SPUwriteDMA
#define SPU_readDMA          SPUreadDMA
#define SPU_writeDMAMem      SPUwriteDMAMem
#define SPU_readDMAMem       SPUreadDMAMem
#define SPU_playADPCMchannel SPUplayADPCMchannel
#define SPU_freeze           SPUfreeze
#define SPU_registerCallback SPUregisterCallback
#define SPU_async            SPUasync
#define SPU_playCDDAchannel  SPUplayCDDAchannel

// PAD Functions
long PADconfigure(void);
void PADabout(void);
long (PADinit)(long);
long (PADshutdown)(void);	
long (PADtest)(void);		
long (PADclose)(void);
long (PADquery)(void);
long (PADreadPort1)(PadDataS*);
long (PADreadPort2)(PadDataS*);
long (PADkeypressed)(void);
unsigned char (PADstartPoll)(int);
unsigned char (PADpoll)(unsigned char);
void (PADsetSensitive)(int);

// PAD function pointers
#define  PAD1_configure PADconfigure
#define PAD1_about PADabout
#define  PAD1_init PADinit
#define  PAD1_shutdown PADshutdown
#define  PAD1_test 	PADtest
#define  PAD1_open 	PADopen
#define  PAD1_close PADclose            
#define  PAD1_query PADquery            
#define PAD1_readPort1 PADreadPort1        
#define  PAD1_keypressed PADkeypressed       
#define  PAD1_startPoll PADstartPoll        
#define  PAD1_poll PADpoll             
#define  PAD1_setSensitive PADsetSensitive     


#define  PAD2_configure PADconfigure
#define PAD2_about PADabout
#define  PAD2_init PADinit
#define PAD2_shutdown PADshutdown
#define PAD2_test PADtest
#define PAD2_open PADopen
#define  PAD2_close PADclose            
#define  PAD2_query PADquery            
#define PAD2_readPort1 PADreadPort1        
#define  PAD2_keypressed PADkeypressed       
#define  PAD2_startPoll PADstartPoll        
#define  PAD2_poll PADpoll             
#define  PAD2_setSensitive PADsetSensitive     




#ifdef ENABLE_SIO1API

// SIO1 Functions (link cable)
typedef long (CALLBACK* SIO1init)(void);
typedef long (CALLBACK* SIO1shutdown)(void);
typedef long (CALLBACK* SIO1close)(void);
typedef long (CALLBACK* SIO1configure)(void);
typedef long (CALLBACK* SIO1test)(void);
typedef void (CALLBACK* SIO1about)(void);
typedef void (CALLBACK* SIO1pause)(void);
typedef void (CALLBACK* SIO1resume)(void);
typedef long (CALLBACK* SIO1keypressed)(int);
typedef void (CALLBACK* SIO1writeData8)(unsigned char);
typedef void (CALLBACK* SIO1writeData16)(unsigned short);
typedef void (CALLBACK* SIO1writeData32)(unsigned long);
typedef void (CALLBACK* SIO1writeStat16)(unsigned short);
typedef void (CALLBACK* SIO1writeStat32)(unsigned long);
typedef void (CALLBACK* SIO1writeMode16)(unsigned short);
typedef void (CALLBACK* SIO1writeMode32)(unsigned long);
typedef void (CALLBACK* SIO1writeCtrl16)(unsigned short);
typedef void (CALLBACK* SIO1writeCtrl32)(unsigned long);
typedef void (CALLBACK* SIO1writeBaud16)(unsigned short);
typedef void (CALLBACK* SIO1writeBaud32)(unsigned long);
typedef unsigned char (CALLBACK* SIO1readData8)(void);
typedef unsigned short (CALLBACK* SIO1readData16)(void);
typedef unsigned long (CALLBACK* SIO1readData32)(void);
typedef unsigned short (CALLBACK* SIO1readStat16)(void);
typedef unsigned long (CALLBACK* SIO1readStat32)(void);
typedef unsigned short (CALLBACK* SIO1readMode16)(void);
typedef unsigned long (CALLBACK* SIO1readMode32)(void);
typedef unsigned short (CALLBACK* SIO1readCtrl16)(void);
typedef unsigned long (CALLBACK* SIO1readCtrl32)(void);
typedef unsigned short (CALLBACK* SIO1readBaud16)(void);
typedef unsigned long (CALLBACK* SIO1readBaud32)(void);
typedef void (CALLBACK* SIO1registerCallback)(void (CALLBACK *callback)(void));

// SIO1 function pointers 
extern SIO1init               SIO1_init;
extern SIO1shutdown           SIO1_shutdown;
extern SIO1open               SIO1_open;
extern SIO1close              SIO1_close; 
extern SIO1test               SIO1_test;
extern SIO1configure          SIO1_configure;
extern SIO1about              SIO1_about;
extern SIO1pause              SIO1_pause;
extern SIO1resume             SIO1_resume;
extern SIO1keypressed         SIO1_keypressed;
extern SIO1writeData8         SIO1_writeData8;
extern SIO1writeData16        SIO1_writeData16;
extern SIO1writeData32        SIO1_writeData32;
extern SIO1writeStat16        SIO1_writeStat16;
extern SIO1writeStat32        SIO1_writeStat32;
extern SIO1writeMode16        SIO1_writeMode16;
extern SIO1writeMode32        SIO1_writeMode32;
extern SIO1writeCtrl16        SIO1_writeCtrl16;
extern SIO1writeCtrl32        SIO1_writeCtrl32;
extern SIO1writeBaud16        SIO1_writeBaud16;
extern SIO1writeBaud32        SIO1_writeBaud32;
extern SIO1readData8          SIO1_readData8;
extern SIO1readData16         SIO1_readData16;
extern SIO1readData32         SIO1_readData32;
extern SIO1readStat16         SIO1_readStat16;
extern SIO1readStat32         SIO1_readStat32;
extern SIO1readMode16         SIO1_readMode16;
extern SIO1readMode32         SIO1_readMode32;
extern SIO1readCtrl16         SIO1_readCtrl16;
extern SIO1readCtrl32         SIO1_readCtrl32;
extern SIO1readBaud16         SIO1_readBaud16;
extern SIO1readBaud32         SIO1_readBaud32;
extern SIO1registerCallback   SIO1_registerCallback;

#endif

void CALLBACK clearDynarec(void);

void SetIsoFile(const char *filename);
const char *GetIsoFile(void);
boolean UsingIso(void);
void SetCdOpenCaseTime(s64 time);

#ifdef __cplusplus
}
#endif
#endif
