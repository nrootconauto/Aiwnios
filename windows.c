#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_pixels.h>
static SDL_Palette *sdl_p;
static SDL_Surface *screen;
static SDL_Window *window;
static SDL_Rect view_port;
static SDL_Renderer *renderer;
static uint32_t palette[16];
void DrawWindowNew() {
  SDL_Init(SDL_INIT_EVERYTHING);
  window=SDL_CreateWindow("AIWNIOS",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,640,480,SDL_WINDOW_SHOWN|SDL_WINDOW_RESIZABLE);
  screen=SDL_CreateRGBSurface(0,640,480,8,0,0,0,0);
  SDL_SetWindowMinimumSize(window,640,480);
  SDL_SetSurfacePalette(screen,sdl_p=SDL_AllocPalette(256));
  renderer=SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED);
}
static void UpdateViewPort() {
  int w,h,margin_x=0,margin_y=0,w2,h2;
  SDL_GetWindowSize(window,&w,&h);
  if(w>h) {
    h2=w*480./640;
    w2=w;
    margin_y=(h-h2)/2;
    if(h<h2) {
      margin_y=0;
      goto use_h;
    }
  } else {
use_h:
    w2=h*640./480;
    h2=h;
    margin_x=(w-w2)/2;
  }
  view_port.x=margin_x;
  view_port.y=margin_y;
  view_port.w=w2;
  view_port.h=h2;
}
void UpdateScreen(char *px,int64_t w,int64_t h,int64_t w_internal) {
  int64_t idx;
  SDL_Texture *text;
  SDL_LockSurface(screen);
  char *dst=screen->pixels;
  for(idx=0;idx!=h;idx++) {
    memcpy(dst,px,w);
    px+=w_internal;
    dst+=screen->pitch;
  }
  SDL_UnlockSurface(screen);
  SDL_RenderClear(renderer);
  SDL_PollEvent(NULL);
  UpdateViewPort();
  text=SDL_CreateTextureFromSurface(renderer,screen);
  SDL_RenderCopy(renderer,text,NULL,&view_port);
  SDL_RenderPresent(renderer);
  SDL_DestroyTexture(text);
}

void GrPaletteColorSet(int64_t i,uint64_t bgr48) {
  int64_t b=(bgr48&0xffff)/(double)0xffff*0xff;
  int64_t g=((bgr48>>16)&0xffff)/(double)0xffff*0xff;
  int64_t r=((bgr48>>32)&0xffff)/(double)0xffff*0xff;
  palette[i]=r|(g<<8)|(b<<16);
  SDL_Color c={r,g,b,0xff};
  SDL_SetPaletteColors(sdl_p,&c,i,1);
}
