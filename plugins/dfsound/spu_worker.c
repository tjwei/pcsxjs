/***************************************************************************
                            spu.c  -  description
                             -------------------
    begin                : Wed May 15 2002
    copyright            : (C) 2002 by Pete Bernert
    email                : BlackDove@addcom.de
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version. See also the license.txt file for *
 *   additional informations.                                              *
 *                                                                         *
 ***************************************************************************/

#include "stdafx.h"

#define _IN_SPU

#include "externals.h"
#include "cfg.h"
#include "dsoundoss.h"
#include "regs.h"

#ifdef ENABLE_NLS
#include <libintl.h>
#include <locale.h>
#define _(x)  gettext(x)
#define N_(x) (x)
#else
#define _(x)  (x)
#define N_(x) (x)
#endif

#if defined (USEMACOSX)
static char * libraryName     = N_("Mac OS X Sound");
#elif defined (USEALSA)
static char * libraryName     = N_("ALSA Sound");
#elif defined (USEOSS)
static char * libraryName     = N_("OSS Sound");
#elif defined (USESDLSOUND)
static char * libraryName     = N_("SDL Sound");
#elif defined (USEPULSEAUDIO)
static char * libraryName     = N_("PulseAudio Sound");
#else
static char * libraryName     = N_("NULL Sound");
#endif

static char * libraryInfo     = N_("P.E.Op.S. Sound Driver V1.7\nCoded by Pete Bernert and the P.E.Op.S. team\n");

// globals

// psx buffer / addresses

unsigned short  regArea[10000];
unsigned short  spuMem[256*1024];
unsigned char * spuMemC;
unsigned char * pSpuIrq=0;
unsigned char * pSpuBuffer;
unsigned char * pMixIrq=0;

// user settings

int             iVolume=3;
int             iXAPitch=1;
int             iUseTimer=2;
int             iSPUIRQWait=1;
int             iSoundDebugMode=0;
int             iRecordMode=0;
int             iUseReverb=2;
int             iUseInterpolation=2;
int             iDisStereo=0;

// MAIN infos struct for each channel

SPUCHAN         s_chan[MAXCHAN+1];                     // channel + 1 infos (1 is security for fmod handling)
REVERBInfo      rvb;

unsigned long   dwNoiseVal=1;                          // global noise generator
int             iSpuAsyncWait=0;

unsigned short  spuCtrl=0;                             // some vars to store psx reg infos
unsigned short  spuStat=0;
unsigned short  spuIrq=0;
unsigned long   spuAddr=0xffffffff;                    // address into spu mem
int             bEndThread=0;                          // thread handlers
int             bThreadEnded=0;
int             bSpuInit=0;
int             bSPUIsOpen=0;
#ifdef PTHREAD
static pthread_t thread = (pthread_t)-1;               // thread id (linux)
#endif

unsigned long dwNewChannel=0;                          // flags for faster testing, if new channel starts

void (CALLBACK *irqCallback)(void)=0;                  // func of main emu, called on spu irq
void (CALLBACK *cddavCallback)(unsigned short,unsigned short)=0;

// certain globals (were local before, but with the new timeproc I need em global)

static const int f[5][2] = {   {    0,  0  },
                        {   60,  0  },
                        {  115, -52 },
                        {   98, -55 },
                        {  122, -60 } };
int SSumR[NSSIZE];
int SSumL[NSSIZE];
int iFMod[NSSIZE];
int iCycle = 0;
short * pS;

int lastch=-1;             // last channel processed on spu irq in timer mode
static int lastns=0;       // last ns pos
static int iSecureStart=0; // secure start counter

////////////////////////////////////////////////////////////////////////
// CODE AREA
////////////////////////////////////////////////////////////////////////

// dirty inline func includes

#include "reverb.c"
#include "adsr.c"

////////////////////////////////////////////////////////////////////////
// helpers for simple interpolation

//
// easy interpolation on upsampling, no special filter, just "Pete's common sense" tm
//
// instead of having n equal sample values in a row like:
//       ____
//           |____
//
// we compare the current delta change with the next delta change.
//
// if curr_delta is positive,
//
//  - and next delta is smaller (or changing direction):
//         \.
//          -__
//
//  - and next delta significant (at least twice) bigger:
//         --_
//            \.
//
//  - and next delta is nearly same:
//          \.
//           \.
//
//
// if curr_delta is negative,
//
//  - and next delta is smaller (or changing direction):
//          _--
//         /
//
//  - and next delta significant (at least twice) bigger:
//            /
//         __- 
//
//  - and next delta is nearly same:
//           /
//          /
//


#include "gauss_i.h"

////////////////////////////////////////////////////////////////////////

#include "xa.c"
#include <emscripten.h>


#define PAUSE_W 5
#define PAUSE_L 5000

////////////////////////////////////////////////////////////////////////

extern int isMute;
void CALLBACK SPUasync(unsigned long cycle)
{
 if(isMute){
     printf("mute....\n");
    return;
 }
 //printf("spuasync %d %d\n", bSpuInit, iSpuAsyncWait);
 if(iSpuAsyncWait)
  {
   iSpuAsyncWait++;
   if(iSpuAsyncWait<=64) return;
   iSpuAsyncWait=0;
  }

   // MAINThread(0);                                      // -> linux high-compat mode

}

// SPU UPDATE... new epsxe func
//  1 time every 32 hsync lines
//  (312/32)x50 in pal
//  (262/32)x60 in ntsc

// since epsxe 1.5.2 (linux) uses SPUupdate, not SPUasync, I will
// leave that func in the linux port, until epsxe linux is using
// the async function as well

void CALLBACK SPUupdate(void)
{
 SPUasync(0);
}

// XA AUDIO

void CALLBACK SPUplayADPCMchannel(xa_decode_t *xap)
{
 if(!xap)       return;
 if(!xap->freq) return;                                // no xa freq ? bye
 //printf("adpcm %p %d\n", xap, sizeof(*xap));
 EM_ASM_({
     SPUplayADPCMchannel($0);
     }, xap );
 //FeedXA(xap);                                          // call main XA feeder
}

// CDDA AUDIO
void CALLBACK SPUplayCDDAchannel(short *pcm, int nbytes)
{
 if (!pcm)      return;
 if (nbytes<=0) return;
 printf("cdda %p %d\n", pcm, nbytes);

 //FeedCDDA((unsigned char *)pcm, nbytes);
}


// INIT/EXIT STUFF

// SPUINIT: this func will be called first by the main emu
long CALLBACK SPUinit(void)
{

 return 0;
}

// SPUOPEN: called by main emu after init
long CALLBACK SPUopen(void)
{
 return PSE_SPU_ERR_SUCCESS;
}

// SPUCLOSE: called before shutdown
long CALLBACK SPUclose(void)
{
 return 0;
}

// SPUSHUTDOWN: called by main emu on final exit
long CALLBACK SPUshutdown(void)
{
 return 0;
}

// SPUTEST: we don't test, we are always fine ;)
long CALLBACK SPUtest(void)
{
 return 0;
}

// SPUCONFIGURE: call config dialog
long CALLBACK SPUconfigure(void)
{
 return 0;
}

// SPUABOUT: show about window
void CALLBACK SPUabout(void)
{
}

// SETUP CALLBACKS
// this functions will be called once, 
// passes a callback that should be called on SPU-IRQ/cdda volume change
void CALLBACK SPUregisterCallback(void (CALLBACK *callback)(void))
{
 irqCallback = callback;
}

void CALLBACK SPUregisterCDDAVolume(void (CALLBACK *CDDAVcallback)(unsigned short,unsigned short))
{
 cddavCallback = CDDAVcallback;
}

// COMMON PLUGIN INFO FUNCS
char * CALLBACK PSEgetLibName(void)
{
 return _(libraryName);
}

unsigned long CALLBACK PSEgetLibType(void)
{
 return  PSE_LT_SPU;
}

unsigned long CALLBACK PSEgetLibVersion(void)
{
 return (1 << 16) | (6 << 8);
}

char * SPUgetLibInfos(void)
{
 return _(libraryInfo);
}
void * get_SPU_ptr(int i){
    if(i==0){
        return regArea;
    }
    return s_chan;
}