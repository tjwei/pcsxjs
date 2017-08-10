#include <emscripten.h>
#include <stdlib.h>
#include <SDL/SDL.h>
unsigned char psxVub[2*1024*1024];
unsigned short *psxVuw;
static SDL_Surface *sdl_display;
static SDL_Surface *sdl_ximage;
SDL_Rect srcrect;
SDL_Rect dstrect;
void BlitSDL32(SDL_Surface *surface, int x, int y, int dx, int dy, int rgb24)
{
 unsigned char *pD;
 unsigned int startxy;
 uint32_t lu, lr, lg, lb;
 unsigned short s;
 unsigned short row, column;

 int32_t lPitch = surface->pitch;//PSXDisplay.DisplayMode.x << 2;

 uint32_t *destpix;

 if (SDL_MUSTLOCK(surface)) SDL_LockSurface(surface);
 if (rgb24)
  {
   for (column = 0; column < dy; column++)
    {      
     startxy = ((1024) * (column + y)) + x;
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
     startxy = (1024 * (column + y)) + x;
     destpix = (uint32_t *)(surface->pixels + (column * lPitch));
     for (row = 0; row < dx; row++)
      {
       //printf("blt row %d\n", row);
       s = psxVuw[startxy++];
      destpix[row] = 
          (((s << 3) & 0xf8) | ((s << 6) & 0xf800) | ((s << 9) & 0xf80000)) | 0xff000000;
      //printf("blt d %x\n", destpix[row]);
      }
    }
  }
  if (SDL_MUSTLOCK(surface)) SDL_UnlockSurface(surface);
    //printf("blt 3\n");
}

void render(int x, int y, int dx, int dy, int rgb24)
{ 
 //printf("render %d %d %d %d %d\n", x,y,dx,dy, rgb24);
  BlitSDL32(sdl_ximage, x, y, dx, dy, rgb24);
  SDL_Flip(sdl_ximage);
  dstrect.x=0;
  dstrect.y=0;
  dstrect.w=640;
  dstrect.h=480;
  srcrect.x=0;
  srcrect.y=0;
  srcrect.w=dx;
  srcrect.h=dy;
  SDL_BlitScaled(sdl_ximage, &srcrect, sdl_display, &dstrect);
  SDL_Flip(sdl_display);
}
void *var_ptrs[]={
	psxVub
};
void * get_ptr(int i){
	return var_ptrs[i];
}
extern void LoadPADConfig();
extern void InitKeyboard();
int main(){
//	if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK|SDL_INIT_AUDIO)<0)   
	if(SDL_Init(SDL_INIT_VIDEO)<0)   
	  printf ("(x) Failed to Init SDL!!!\n");	  
	else {
  		printf("sdl init ok\n");
  		sdl_display = SDL_SetVideoMode(640,480, 32,SDL_HWSURFACE);
  		sdl_ximage= SDL_CreateRGBSurface(SDL_HWSURFACE,640,480,32,0x00ff0000,0x0000ff00,0x000000ff,0);
	}
	//psxVub=malloc(2*1024*1024);
	psxVuw = (unsigned short *) psxVub;
	LoadPADConfig();
	InitKeyboard();
	emscripten_exit_with_live_runtime();
}