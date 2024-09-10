#pragma once
#include <stdint.h>
extern int64_t sdl_window_grab_enable;
void DrawWindowNew();
void UpdateScreen(char *px, int64_t w, int64_t h, int64_t wi);
void GrPaletteColorSet(int64_t i, uint64_t bgr48);
int64_t ScreenUpdateInProgress();
void SetCaptureMouse(int64_t);
void LaunchSDL(void (*boot_ptr)(void *data), void *data);
void WaitForSDLQuit();
void SetKBCallback(void *fptr);
void SetMSCallback(void *fptr);
void DeinitVideo();
