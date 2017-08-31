/***************************************************************************
                          draw.c  -  description
                             -------------------
    begin                : Sun Oct 28 2001
    copyright            : (C) 2001 by Pete Bernert
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

#define _IN_DRAW

#include "externals.h"
#include "gpu.h"
#include "draw.h"
#include "prim.h"
#include "menu.h"
#include "interp.h"
#include "swap.h"

// misc globals
int            iResX;
int            iResY;
long           lLowerpart;
BOOL           bIsFirstFrame = TRUE;
BOOL           bCheckMask = FALSE;
unsigned short sSetMask = 0;
unsigned long  lSetMask = 0;
int            iDesktopCol = 16;
int            iShowFPS = 0;
int            iWinSize; 
int            iMaintainAspect = 0;
int            iUseNoStretchBlt = 0;
int            iFastFwd = 0;
int            iDebugMode = 0;
int            iFVDisplay = 0;
PSXPoint_t     ptCursorPoint[8];
unsigned short usCursorActive = 0;
char *               pCaptionText;
//unsigned int   LUT16to32[65536];
//unsigned int   RGBtoYUV[65536];

void DestroyDisplay(void)
{


}

static int depth=32;
int root_window_id=0;


// Create display

void CreateDisplay(void)
{
}


#include <emscripten.h>
void DoBufferSwap(void)
{  
   // printf("%d %d %d %d\n", PSXDisplay.DisplayPosition.x, PSXDisplay.DisplayPosition.y, PreviousPSXDisplay.Range.x1, PreviousPSXDisplay.DisplayMode.y);
   EM_ASM_({render($0,$1,$2,$3,$4,$5,$6);},
   PSXDisplay.DisplayPosition.x,
   PSXDisplay.DisplayPosition.y,
   PreviousPSXDisplay.Range.x1,
   PreviousPSXDisplay.DisplayMode.y,
   PSXDisplay.DisplayMode.x,
   PSXDisplay.DisplayMode.y,
   PSXDisplay.RGB24
   );
}

void DoClearScreenBuffer(void)                         // CLEAR DX BUFFER
{

}

void DoClearFrontBuffer(void)                          // CLEAR DX BUFFER
{
}

unsigned long ulInitDisplay(void)
{                                   // x stuff
 return (unsigned long) 1;
}

void CloseDisplay(void)
{
 DestroyDisplay();
}
extern unsigned long SoundGetBytesBuffered_value;
int isMute=0;
void *get_PadState_ptr();
void * params_ptrs[] ={&(PSXDisplay.DisplayPosition.x), 
                    &(PSXDisplay.DisplayPosition.y),
                    &(PreviousPSXDisplay.Range.x1),
                    &(PreviousPSXDisplay.DisplayMode.y),
                    &PSXDisplay.RGB24,
                    &PSXDisplay.DisplayMode.x,
                    &PSXDisplay.DisplayMode.y,
                    &SoundGetBytesBuffered_value,
                    &isMute,                      
                    };
void * get_ptr(int i){
    if(i==-1)   return psxVub;    
    if(i==-2) return get_PadState_ptr();
    return params_ptrs[i];
}


void ShowGpuPic(void)
{
}

void ShowTextGpuPic(void)
{
}
