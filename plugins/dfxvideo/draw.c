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

//unsigned int   LUT16to32[65536];
//unsigned int   RGBtoYUV[65536];

#include <SDL/SDL.h>

#include <sys/ipc.h>
#include <sys/shm.h>

int xv_port = -1;
int xv_id = -1;
int xv_depth = 0;
int yuv_port = -1;
int yuv_id = -1;
int use_yuv = 0;
int xv_vsync = 0;

int finalw,finalh;


#include <time.h>


static SDL_Surface *sdl_display;
static SDL_Surface *sdl_ximage;



char *               Xpixels;
char *               pCaptionText;

static int fx=0;



// close display

void DestroyDisplay(void)
{
  SDL_FreeSurface(sdl_ximage);
  SDL_FreeSurface(sdl_display);
  SDL_Quit();

}

static int depth=32;
int root_window_id=0;


// Create display
SDL_Rect srcrect;
SDL_Rect dstrect;
void CreateDisplay(void)
{
if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK|SDL_INIT_AUDIO)<0)
   {
	  printf ("(x) Failed to Init SDL!!!\n");	  
   }
else {
  printf("gpu sdl init ok\n");
  sdl_display = SDL_SetVideoMode(iResX,iResY, depth,SDL_HWSURFACE);
  sdl_ximage= SDL_CreateRGBSurface(SDL_HWSURFACE,iResX,iResY,depth,0x00ff0000,0x0000ff00,0x000000ff,0);
}
 
}


void BlitSDL32(SDL_Surface *surface, int32_t x, int32_t y)
{
 unsigned char *pD;
 unsigned int startxy;
 uint32_t lu, lr, lg, lb;
 unsigned short s;
 unsigned short row, column;
 unsigned short dx = PreviousPSXDisplay.Range.x1;
 unsigned short dy = PreviousPSXDisplay.DisplayMode.y;

 int32_t lPitch = surface->pitch;//PSXDisplay.DisplayMode.x << 2;

 uint32_t *destpix;
/*
 if (PreviousPSXDisplay.Range.y0) // centering needed?
  {
   memset(surf, 0, (PreviousPSXDisplay.Range.y0 >> 1) * lPitch);

   dy -= PreviousPSXDisplay.Range.y0;
   surf += (PreviousPSXDisplay.Range.y0 >> 1) * lPitch;

   memset(surf + dy * lPitch,
          0, ((PreviousPSXDisplay.Range.y0 + 1) >> 1) * lPitch);
  }
  

 if (PreviousPSXDisplay.Range.x0)
  {
   for (column = 0; column < dy; column++)
    {
     destpix = (uint32_t *)(surf + (column * lPitch));
     memset(destpix, 0, PreviousPSXDisplay.Range.x0 << 2);
    }
   surf += PreviousPSXDisplay.Range.x0 << 2;
  }
  */
 if (SDL_MUSTLOCK(surface)) SDL_LockSurface(surface);
 if (PSXDisplay.RGB24)
  {
  //printf("blt 1\n");
   for (column = 0; column < dy; column++)
    {      
     startxy = ((1024) * (column + y)) + x;
     //pD = (unsigned char *)&psxVuw[startxy];
     pD = &psxVub[startxy*2];
     destpix = (uint32_t *)(surface->pixels + (column * lPitch));
     for (row = 0; row < dx; row++)
      {
      //printf("column %d %d %d\n",column, dy, row);
      
       lr = pD[0];
       lg = pD[1];
       lb = pD[2];
       #ifdef __EMSCRIPTEN__
       destpix[row] = 
          0xff000000 | (lb << 16) | (lg << 8) | (lr);
       #else
       destpix[row] = 
          0xff000000 | (lr << 16) | (lg << 8) | (lb);
      #endif

       pD += 3;
      }
    }
  }
 else
  {
    //printf("blt 2\n");
   for (column = 0;column<dy;column++)
    {
    //printf("blt column %d\n",column);
     startxy = (1024 * (column + y)) + x;
    //printf("blt startxy %d\n", startxy);
     destpix = (uint32_t *)(surface->pixels + (column * lPitch));
     for (row = 0; row < dx; row++)
      {
       //printf("blt row %d\n", row);
       s = GETLE16(&psxVuw[startxy++]);
      //printf("blt s %d\n", s);
      #ifdef __EMSCRIPTEN__
      destpix[row] = 
          (((s << 3) & 0xf8) | ((s << 6) & 0xf800) | ((s << 9) & 0xf80000)) | 0xff000000;
      #else
       destpix[row] = 
          (((s << 19) & 0xf80000) | ((s << 6) & 0xf800) | ((s >> 7) & 0xf8)) | 0xff000000;
      #endif
      //printf("blt d %x\n", destpix[row]);
      }
    }
  }
  if (SDL_MUSTLOCK(surface)) SDL_UnlockSurface(surface);
    //printf("blt 3\n");
}
extern time_t tStart;

//Note: dest x,y,w,h are both input and output variables
inline void MaintainAspect(unsigned int *dx,unsigned int *dy,unsigned int *dw,unsigned int *dh)
{
	//Currently just 4/3 aspect ratio
	int t;

	if (*dw * 3 > *dh * 4) {
		t = *dh * 4.0f / 3;	//new width aspect
		*dx = (*dw - t) / 2;	//centering
		*dw = t;
	} else {
		t = *dw * 3.0f / 4;
		*dy = (*dh - t) / 2;
		*dh = t;
	}
}


void DoBufferSwap(void)
{  
#ifdef __EMSCRIPTEN__
  BlitSDL32(sdl_ximage, PSXDisplay.DisplayPosition.x, PSXDisplay.DisplayPosition.y);
  SDL_Flip(sdl_ximage);
  dstrect.x=0;
  dstrect.y=0;
  dstrect.w=iResX;
  dstrect.h=iResY;
  srcrect.x=0;
  srcrect.y=0;
  srcrect.w=PSXDisplay.DisplayMode.x;
  srcrect.h=PSXDisplay.DisplayMode.y;
  SDL_BlitScaled(sdl_ximage, &srcrect, sdl_display, &dstrect);
#else
  BlitSDL32(sdl_display, PSXDisplay.DisplayPosition.x, PSXDisplay.DisplayPosition.y);
#endif
  SDL_Flip(sdl_display);
}

void DoClearScreenBuffer(void)                         // CLEAR DX BUFFER
{

}

void DoClearFrontBuffer(void)                          // CLEAR DX BUFFER
{
}

unsigned long ulInitDisplay(void)
{
 CreateDisplay();                                      // x stuff
 return (unsigned long) 1;
}

void CloseDisplay(void)
{
 DestroyDisplay();
}




void ShowGpuPic(void)
{
}

void ShowTextGpuPic(void)
{
}
