#include "c/aiwn_asm.h"
#include "c/aiwn_logo.h"
#include "c/aiwn_mem.h"
#include "c/aiwn_snd.h"
#include "c/aiwn_tui.h"
#include "c/aiwn_windows.h"
#include "c/lzw.h"
#include <SDL.h>
#include <SDL_pixels.h>
#include <SDL_render.h>
#include <SDL_surface.h>
#include <SDL_video.h>
#include <stdio.h>
#include <string.h>
static SDL_Palette *sdl_p;
static SDL_Surface *screen;
static SDL_Surface *window_icon;
static SDL_Window *window;
static SDL_Rect view_port;
static SDL_Renderer *renderer;
static SDL_Thread *sdl_main_thread;
int64_t user_ev_num;
static SDL_mutex *screen_mutex, *screen_mutex2;
static int64_t screen_ready = 0, screen_update_in_progress = 0;
#define USER_CODE_DRAW_WIN_NEW 1
#define USER_CODE_UPDATE       2
void DeinitVideo() {
  if (screen_mutex)
    SDL_LockMutex(screen_mutex);
  if (renderer)
    SDL_DestroyRenderer(renderer);
  if (window)
    SDL_DestroyWindow(window);
}
static void _DrawWindowNew() {
  int64_t row;
  uint8_t logo[0x10000];
  lzw_decompress(aiwnios_logo.compressed_pixel_data,        //
                 sizeof aiwnios_logo.compressed_pixel_data, //
                 logo, sizeof logo);
  SDL_Surface *window_icon_proto = SDL_CreateRGBSurfaceWithFormat(
      0, aiwnios_logo.width, aiwnios_logo.height,
      aiwnios_logo.bytes_per_pixel * 8, SDL_PIXELFORMAT_ABGR8888);
  if (!window_icon_proto)
    goto err_wincon;
  SDL_LockSurface(window_icon_proto);
  for (row = 0; row != window_icon_proto->h; row++) {
    memcpy((char *)window_icon_proto->pixels + row * window_icon_proto->pitch,
           logo + 4 * aiwnios_logo.width * row, aiwnios_logo.width * 4);
  }
  SDL_UnlockSurface(window_icon_proto);
  window_icon =
      SDL_ConvertSurfaceFormat(window_icon_proto, SDL_PIXELFORMAT_RGB888, 0);
  if (!window_icon) {
  err_wincon:
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "AIWNIOS",
                             "Failed to make window icon.", NULL);
    exit(EXIT_FAILURE);
  }

  SDL_FreeSurface(window_icon_proto);
  SDL_SetHintWithPriority(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0",
                          SDL_HINT_OVERRIDE);
   SDL_SetHintWithPriority(SDL_HINT_RENDER_SCALE_QUALITY, "linear",
                           SDL_HINT_OVERRIDE);
  SDL_SetHintWithPriority(SDL_HINT_ALLOW_ALT_TAB_WHILE_GRABBED, "1",
                          SDL_HINT_OVERRIDE);
  SDL_RendererInfo info;
  screen_mutex = SDL_CreateMutex();
  screen_mutex2 = SDL_CreateMutex();
  SDL_LockMutex(screen_mutex);
  window = SDL_CreateWindow("AIWNIOS", SDL_WINDOWPOS_UNDEFINED,
                            SDL_WINDOWPOS_UNDEFINED, 640, 480,
                            SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
  if (!window) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "AIWNIOS",
                             "Failed to create window.", NULL);
    exit(EXIT_FAILURE);
  }
  SDL_SetWindowIcon(window, window_icon);
  SDL_SetWindowKeyboardGrab(window,
                            sdl_window_grab_enable ? SDL_TRUE : SDL_FALSE);
  screen = SDL_CreateRGBSurface(0, 640, 480, 8, 0, 0, 0, 0);
  if (!screen) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "AIWNIOS",
                             "Failed to create surface.", NULL);
    exit(EXIT_FAILURE);
  }
  SDL_SetWindowMinimumSize(window, 640, 480);
  SDL_ShowCursor(SDL_DISABLE);
  renderer = SDL_CreateRenderer(window, -1, 0);
  if (!renderer) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "AIWNIOS",
                             "Failed to create renderer.", NULL);
    exit(EXIT_FAILURE);
  }
  SDL_UnlockMutex(screen_mutex);
  Misc_LBts(&screen_ready, 0);
}

static void UpdateViewPort() {
  int w, h, margin_x = 0, margin_y = 0, w2, h2;
  SDL_GetWindowSize(window, &w, &h);
  float ratio = 640. / 480;
  if (w > h * ratio) {
    h2 = h;
    w2 = ratio * h;
    margin_x = (w - w2) / 2;
  } else {
    h2 = w / ratio;
    w2 = w;
    margin_y = (h - h2) / 2;
  }
  view_port.x = margin_x;
  view_port.y = margin_y;
  view_port.w = w2;
  view_port.h = h2;
}

void DrawWindowNew() {
  SDL_Event event;
  memset(&event, 0, sizeof event);
  event.user.code = USER_CODE_DRAW_WIN_NEW;
  event.type = user_ev_num;
  SDL_PushEvent(&event);
  while (!Misc_Bt(&screen_ready, 0))
    SDL_Delay(1);
  return;
}
void UpdateScreen(char *px, int64_t w, int64_t h, int64_t w_internal) {
  SDL_Event event;
  Misc_LBts(&screen_update_in_progress, 0);
  memset(&event, 0, sizeof event);
  event.user.code = USER_CODE_UPDATE;
  event.user.data1 = px;
  event.user.data2 = w_internal;
  event.type = user_ev_num;
  SDL_LockMutex(screen_mutex);
  SDL_PushEvent(&event);
  SDL_UnlockMutex(screen_mutex);
  return;
}
int64_t ScreenUpdateInProgress() {
  return Misc_Bt(&screen_update_in_progress, 0);
}
static void _UpdateScreen(char *px, int64_t w, int64_t h, int64_t w_internal) {
  int64_t idx;
  SDL_Texture *text;
  SDL_LockSurface(screen);
  char *dst = screen->pixels;
  for (idx = 0; idx != h; idx++) {
    memcpy(dst, px, w);
    px += w_internal;
    dst += screen->pitch;
  }
  SDL_UnlockSurface(screen);
  SDL_RenderClear(renderer);
  UpdateViewPort();
  text = SDL_CreateTextureFromSurface(renderer, screen);
  if (!text) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "AIWNIOS",
                             "Failed to create texture.", NULL);
    exit(EXIT_FAILURE);
  }
  SDL_RenderCopy(renderer, text, NULL, &view_port);
  SDL_RenderPresent(renderer);
  SDL_DestroyTexture(text);
  Misc_LBtr(&screen_update_in_progress, 0);
}

void GrPaletteColorSet(int64_t i, uint64_t bgr48) {
  if (!screen)
    return;
  union {
    uint64_t i;
    struct {
      uint16_t b, g, r, pad
    };
  } u = {.i = bgr48};
  uint8_t b = u.b / (double)0xffff * 0xff, g = u.g / (double)0xffff * 0xff,
          r = u.r / (double)0xffff * 0xff;
  SDL_Color c = {r, g, b, 0};
  SDL_LockSurface(screen);
  // I will repeat the color to simulate ignoring the upper 4 bits
  for (int i2 = 0; i2 < 16; i2++)
    SDL_SetPaletteColors(screen->format->palette, &c, i2 * 16 + i, 1);
  SDL_UnlockSurface(screen);
}

#define CH_CTRLA       0x01
#define CH_CTRLB       0x02
#define CH_CTRLC       0x03
#define CH_CTRLD       0x04
#define CH_CTRLE       0x05
#define CH_CTRLF       0x06
#define CH_CTRLG       0x07
#define CH_CTRLH       0x08
#define CH_CTRLI       0x09
#define CH_CTRLJ       0x0A
#define CH_CTRLK       0x0B
#define CH_CTRLL       0x0C
#define CH_CTRLM       0x0D
#define CH_CTRLN       0x0E
#define CH_CTRLO       0x0F
#define CH_CTRLP       0x10
#define CH_CTRLQ       0x11
#define CH_CTRLR       0x12
#define CH_CTRLS       0x13
#define CH_CTRLT       0x14
#define CH_CTRLU       0x15
#define CH_CTRLV       0x16
#define CH_CTRLW       0x17
#define CH_CTRLX       0x18
#define CH_CTRLY       0x19
#define CH_CTRLZ       0x1A
#define CH_CURSOR      0x05
#define CH_BACKSPACE   0x08
#define CH_ESC         0x1B
#define CH_SHIFT_ESC   0x1C
#define CH_SHIFT_SPACE 0x1F
#define CH_SPACE       0x20

// Scan code flags
#define SCf_E0_PREFIX 7
#define SCf_KEY_UP    8
#define SCf_SHIFT     9
#define SCf_CTRL      10
#define SCf_ALT       11
#define SCf_CAPS      12
#define SCf_NUM       13
#define SCf_SCROLL    14
#define SCf_NEW_KEY   15
#define SCf_MS_L_DOWN 16
#define SCf_MS_R_DOWN 17
#define SCf_DELETE    18
#define SCf_INS       19
#define SCf_NO_SHIFT  30
#define SCf_KEY_DESC  31
#define SCF_E0_PREFIX (1 << SCf_E0_PREFIX)
#define SCF_KEY_UP    (1 << SCf_KEY_UP)
#define SCF_SHIFT     (1 << SCf_SHIFT)
#define SCF_CTRL      (1 << SCf_CTRL)
#define SCF_ALT       (1 << SCf_ALT)
#define SCF_CAPS      (1 << SCf_CAPS)
#define SCF_NUM       (1 << SCf_NUM)
#define SCF_SCROLL    (1 << SCf_SCROLL)
#define SCF_NEW_KEY   (1 << SCf_NEW_KEY)
#define SCF_MS_L_DOWN (1 << SCf_MS_L_DOWN)
#define SCF_MS_R_DOWN (1 << SCf_MS_R_DOWN)
#define SCF_DELETE    (1 << SCf_DELETE)
#define SCF_INS       (1 << SCf_INS)
#define SCF_NO_SHIFT  (1 << SCf_NO_SHIFT)
#define SCF_KEY_DESC  (1 << SCf_KEY_DESC)

// TempleOS places a 1 in bit 7 for
// keys with an E0 prefix.
// See \dLK,"::/Doc/CharOverview.DD"\d and \dLK,"KbdHndlr",A="MN:KbdHndlr"\d().
#define SC_ESC          0x01
#define SC_BACKSPACE    0x0E
#define SC_TAB          0x0F
#define SC_ENTER        0x1C
#define SC_SHIFT        0x2A
#define SC_CTRL         0x1D
#define SC_ALT          0x38
#define SC_CAPS         0x3A
#define SC_NUM          0x45
#define SC_SCROLL       0x46
#define SC_CURSOR_UP    0x48
#define SC_CURSOR_DOWN  0x50
#define SC_CURSOR_LEFT  0x4B
#define SC_CURSOR_RIGHT 0x4D
#define SC_PAGE_UP      0x49
#define SC_PAGE_DOWN    0x51
#define SC_HOME         0x47
#define SC_END          0x4F
#define SC_INS          0x52
#define SC_DELETE       0x53
#define SC_F1           0x3B
#define SC_F2           0x3C
#define SC_F3           0x3D
#define SC_F4           0x3E
#define SC_F5           0x3F
#define SC_F6           0x40
#define SC_F7           0x41
#define SC_F8           0x42
#define SC_F9           0x43
#define SC_F10          0x44
#define SC_F11          0x57
#define SC_F12          0x58
#define SC_PAUSE        0x61
#define SC_GUI          0xDB
#define SC_PRTSCRN1     0xAA
#define SC_PRTSCRN2     0xB7

#define IS_CAPS(x) (x & (KMOD_RSHIFT | KMOD_LSHIFT | KMOD_CAPS))
// http://www.rohitab.com/discuss/topic/39438-keyboard-driver/
static char keys[] = {
    0,   CH_ESC, '1',  '2', '3',  '4', '5', '6', '7', '8', '9', '0', '-',
    '=', '\b',   '\t', 'q', 'w',  'e', 'r', 't', 'y', 'u', 'i', 'o', 'p',
    '[', ']',    '\n', 0,   'a',  's', 'd', 'f', 'g', 'h', 'j', 'k', 'l',
    ';', '\'',   '`',  0,   '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',',
    '.', '/',    0,    '*', 0,    ' ', 0,   0,   0,   0,   0,   0,   0,
    0,   0,      0,    0,   0,    0,   0,   0,   0,   '-', 0,   '5', 0,
    '+', 0,      0,    0,   0,    0,   0,   0,   0,   0,   0,   0};
static int64_t K2SC(char ch) {
  int64_t i = 0;
  for (; i != sizeof(keys) / sizeof(*keys); i++) {
    if (keys[i] == ch)
      return i;
  }
}
static int32_t __ScanKey(int64_t *ch, int64_t *sc, SDL_Event *_e) {
  SDL_Event e = *_e;
  int64_t mod = 0, cond, dummy;
  if (!ch)
    ch = &dummy;
  if (!sc)
    sc = &dummy;
  cond = 1;
  if (cond) {
    if (e.type == SDL_KEYDOWN) {
    ent:
      *ch = *sc = 0;
      if (e.key.keysym.mod & (KMOD_LSHIFT | KMOD_RSHIFT))
        mod |= SCF_SHIFT;
      else
        mod |= SCF_NO_SHIFT;
      if (e.key.keysym.mod & (KMOD_LCTRL | KMOD_RCTRL))
        mod |= SCF_CTRL;
      if (e.key.keysym.mod & (KMOD_LALT | KMOD_RALT))
        mod |= SCF_ALT;
      if (e.key.keysym.mod & KMOD_CAPS)
        mod |= SCF_CAPS;
      if (e.key.keysym.mod & KMOD_NUM)
        mod |= SCF_NUM;
      *sc = e.key.keysym.scancode;
      switch (e.key.keysym.scancode) {
      case SDL_SCANCODE_SPACE:
        return *sc = K2SC(' ') | mod;
      case SDL_SCANCODE_APOSTROPHE:
        return *sc = K2SC('\'') | mod;
      case SDL_SCANCODE_COMMA:
        return *sc = K2SC(',') | mod;
      case SDL_SCANCODE_KP_MINUS:
      case SDL_SCANCODE_MINUS:
        return *sc = K2SC('-') | mod;
      case SDL_SCANCODE_KP_PERIOD:
      case SDL_SCANCODE_PERIOD:
        return *sc = K2SC('.') | mod;
      case SDL_SCANCODE_GRAVE:
        return *sc = K2SC('`') | mod;
      case SDL_SCANCODE_KP_DIVIDE:
      case SDL_SCANCODE_SLASH:
        return *sc = K2SC('/') | mod;
      case SDL_SCANCODE_KP_0:
      case SDL_SCANCODE_0:
        return *sc = K2SC('0') | mod;
      case SDL_SCANCODE_KP_1:
      case SDL_SCANCODE_1:
        return *sc = K2SC('1') | mod;
      case SDL_SCANCODE_KP_2:
      case SDL_SCANCODE_2:
        return *sc = K2SC('2') | mod;
      case SDL_SCANCODE_KP_3:
      case SDL_SCANCODE_3:
        return *sc = K2SC('3') | mod;
      case SDL_SCANCODE_KP_4:
      case SDL_SCANCODE_4:
        return *sc = K2SC('4') | mod;
      case SDL_SCANCODE_KP_5:
      case SDL_SCANCODE_5:
        return *sc = K2SC('5') | mod;
      case SDL_SCANCODE_KP_6:
      case SDL_SCANCODE_6:
        return *sc = K2SC('6') | mod;
      case SDL_SCANCODE_KP_7:
      case SDL_SCANCODE_7:
        return *sc = K2SC('7') | mod;
      case SDL_SCANCODE_KP_8:
      case SDL_SCANCODE_8:
        return *sc = K2SC('8') | mod;
      case SDL_SCANCODE_KP_9:
      case SDL_SCANCODE_9:
        return *sc = K2SC('9') | mod;
      case SDL_SCANCODE_SEMICOLON:
        return *sc = K2SC(';') | mod;
      case SDL_SCANCODE_EQUALS:
        return *sc = K2SC('=') | mod;
      case SDL_SCANCODE_LEFTBRACKET:
        return *sc = K2SC('[') | mod;
      case SDL_SCANCODE_RIGHTBRACKET:
        return *sc = K2SC(']') | mod;
      case SDL_SCANCODE_BACKSLASH:
        return *sc = K2SC('\\') | mod;
      case SDL_SCANCODE_Q:
        return *sc = K2SC('q') | mod;
      case SDL_SCANCODE_W:
        return *sc = K2SC('w') | mod;
      case SDL_SCANCODE_E:
        return *sc = K2SC('e') | mod;
      case SDL_SCANCODE_R:
        return *sc = K2SC('r') | mod;
      case SDL_SCANCODE_T:
        return *sc = K2SC('t') | mod;
      case SDL_SCANCODE_Y:
        return *sc = K2SC('y') | mod;
      case SDL_SCANCODE_U:
        return *sc = K2SC('u') | mod;
      case SDL_SCANCODE_I:
        return *sc = K2SC('i') | mod;
      case SDL_SCANCODE_O:
        return *sc = K2SC('o') | mod;
      case SDL_SCANCODE_P:
        return *sc = K2SC('p') | mod;
      case SDL_SCANCODE_A:
        return *sc = K2SC('a') | mod;
      case SDL_SCANCODE_S:
        return *sc = K2SC('s') | mod;
      case SDL_SCANCODE_D:
        return *sc = K2SC('d') | mod;
      case SDL_SCANCODE_F:
        return *sc = K2SC('f') | mod;
      case SDL_SCANCODE_G:
        return *sc = K2SC('g') | mod;
      case SDL_SCANCODE_H:
        return *sc = K2SC('h') | mod;
      case SDL_SCANCODE_J:
        return *sc = K2SC('j') | mod;
      case SDL_SCANCODE_K:
        return *sc = K2SC('k') | mod;
      case SDL_SCANCODE_L:
        return *sc = K2SC('l') | mod;
      case SDL_SCANCODE_Z:
        return *sc = K2SC('z') | mod;
      case SDL_SCANCODE_X:
        return *sc = K2SC('x') | mod;
      case SDL_SCANCODE_C:
        return *sc = K2SC('c') | mod;
      case SDL_SCANCODE_V:
        return *sc = K2SC('v') | mod;
      case SDL_SCANCODE_B:
        return *sc = K2SC('b') | mod;
      case SDL_SCANCODE_N:
        return *sc = K2SC('n') | mod;
      case SDL_SCANCODE_M:
        return *sc = K2SC('m') | mod;
      case SDL_SCANCODE_KP_MULTIPLY:
        return *sc = K2SC('*') | mod;
      case SDL_SCANCODE_KP_PLUS:
        return *sc = K2SC('+') | mod;
      case SDL_SCANCODE_ESCAPE:
        *sc = mod | SC_ESC;
        return 1;
      case SDL_SCANCODE_BACKSPACE:
        *sc = mod | SC_BACKSPACE;
        return 1;
      case SDL_SCANCODE_TAB:
        *sc = mod | SC_TAB;
        return 1;
      case SDL_SCANCODE_KP_ENTER:
      case SDL_SCANCODE_RETURN:
        *sc = mod | SC_ENTER;
        return 1;
      case SDL_SCANCODE_LSHIFT:
      case SDL_SCANCODE_RSHIFT:
        *sc = mod | SC_SHIFT;
        return 1;
      case SDL_SCANCODE_LALT:
        *sc = mod | SC_ALT;
        return 1;
      case SDL_SCANCODE_RALT:
        *sc = mod | SC_ALT;
        return 1;
      case SDL_SCANCODE_LCTRL:
        *sc = mod | SC_CTRL;
        return 1;
      case SDL_SCANCODE_RCTRL:
        *sc = mod | SC_CTRL;
        return 1;
      case SDL_SCANCODE_CAPSLOCK:
        *sc = mod | SC_CAPS;
        return 1;
      case SDL_SCANCODE_NUMLOCKCLEAR:
        *sc = mod | SC_NUM;
        return 1;
      case SDL_SCANCODE_SCROLLLOCK:
        *sc = mod | SC_SCROLL;
        return 1;
      case SDL_SCANCODE_DOWN:
        *sc = mod | SC_CURSOR_DOWN;
        return 1;
      case SDL_SCANCODE_UP:
        *sc = mod | SC_CURSOR_UP;
        return 1;
      case SDL_SCANCODE_RIGHT:
        *sc = mod | SC_CURSOR_RIGHT;
        return 1;
      case SDL_SCANCODE_LEFT:
        *sc = mod | SC_CURSOR_LEFT;
        return 1;
      case SDL_SCANCODE_PAGEDOWN:
        *sc = mod | SC_PAGE_DOWN;
        return 1;
      case SDL_SCANCODE_PAGEUP:
        *sc = mod | SC_PAGE_UP;
        return 1;
      case SDL_SCANCODE_HOME:
        *sc = mod | SC_HOME;
        return 1;
      case SDL_SCANCODE_END:
        *sc = mod | SC_END;
        return 1;
      case SDL_SCANCODE_INSERT:
        *sc = mod | SC_INS;
        return 1;
      case SDL_SCANCODE_DELETE:
        *sc = mod | SC_DELETE;
        return 1;
      case SDL_SCANCODE_LGUI:
      case SDL_SCANCODE_RGUI:
        *sc = mod | SC_GUI;
        return 1;
      case SDL_SCANCODE_PRINTSCREEN:
        *sc = mod | SC_PRTSCRN1;
        return 1;
      case SDL_SCANCODE_PAUSE:
        *sc = mod | SC_PAUSE;
        return 1;
      case SDL_SCANCODE_F1 ... SDL_SCANCODE_F12:
        *sc = mod | (SC_F1 + e.key.keysym.scancode - SDL_SCANCODE_F1);
        return 1;
      }
    } else if (e.type == SDL_KEYUP) {
      mod |= SCF_KEY_UP;
      goto ent;
    }
  }
  return -1;
}
static void (*kb_cb)(int64_t, int64_t);
static void *kb_cb_data;
static int SDLCALL KBCallback(void *d, SDL_Event *e) {
  int64_t c, s;
  if (kb_cb && (-1 != __ScanKey(&c, &s, e))) {
    SetWriteNP(1);
    FFI_CALL_TOS_2(kb_cb, c, s);
  }
  return 0;
}
void SetKBCallback(void *fptr) {
  kb_cb = fptr;
  static init;
  if (!init) {
    init = 1;
    if (IsCmdLineMode2()) {
      TermSetKbCb(fptr, NULL);
    } else
      SDL_AddEventWatch(KBCallback, NULL);
  }
}
// x,y,z,(l<<1)|r
static void (*ms_cb)(int64_t, int64_t, int64_t, int64_t);
static int SDLCALL MSCallback(void *d, SDL_Event *e) {
  static int64_t x, y;
  static int state = 0;
  static int z;
  int x2, y2;
  if (ms_cb)
    switch (e->type) {
    case SDL_MOUSEBUTTONDOWN:
      x = e->button.x, y = e->button.y;
      if (e->button.button == SDL_BUTTON_LEFT)
        state |= 2;
      else
        state |= 1;
      goto ent;
    case SDL_MOUSEBUTTONUP:
      x = e->button.x, y = e->button.y;
      if (e->button.button == SDL_BUTTON_LEFT)
        state &= ~2;
      else
        state &= ~1;
      goto ent;
    case SDL_MOUSEWHEEL:
      z -= e->wheel.y; //???,inverted otherwise
      goto ent;
    case SDL_MOUSEMOTION:
      x = e->motion.x, y = e->motion.y;
    ent:;
      if (x < view_port.x)
        x2 = 0;
      else if (x >= view_port.x + view_port.w)
        x2 = 639;
      else
        x2 = (x - view_port.x) * 640. / view_port.w;
      if (y < view_port.y)
        y2 = 0;
      else if (y >= view_port.y + view_port.h)
        y2 = 479;
      else
        y2 = (y - view_port.y) * 480. / view_port.h;
      SetWriteNP(1);
      if (SDL_GetRelativeMouseMode()) {
        if (e->type != SDL_MOUSEMOTION) // Wierd stuff happens
          FFI_CALL_TOS_4(ms_cb, 640 / 2, 480 / 2, z, state);
        else
          FFI_CALL_TOS_4(ms_cb, e->motion.xrel + 640 / 2,
                         e->motion.yrel + 480 / 2, z, state);
      } else
        FFI_CALL_TOS_4(ms_cb, x2, y2, z, state);
    }
  return 0;
}

void SetMSCallback(void *fptr) {
  ms_cb = fptr;
  static int init;
  if (!init) {
    init = 1;
    if (IsCmdLineMode2()) {
      TermSetMsCb(fptr);
    } else
      SDL_AddEventWatch(MSCallback, NULL);
  }
}

void InputLoop(void *ul) {
  SDL_Event e;
  if (IsCmdLineMode2()) {
    TUIInputLoop(ul);
  } else {
    while (!*(uint64_t *)ul) {
      if (!SDL_WaitEvent(&e))
        continue;
      switch (e.type) {
      case SDL_QUIT:
        return;
      case SDL_USEREVENT:
        switch (e.user.code) {
        case USER_CODE_UPDATE:
          _UpdateScreen(e.user.data1, 640, 480, e.user.data2);
          break;
        case USER_CODE_DRAW_WIN_NEW:
          _DrawWindowNew();
        }
      }
    }
  }
}
void WaitForSDLQuit() {
  SDL_WaitThread(sdl_main_thread, NULL);
}
// Your own your own(used for FPS games)
void SetCaptureMouse(int64_t i) {
  i = i ? SDL_TRUE : SDL_FALSE;
  SDL_SetRelativeMouseMode(i);
  SDL_SetWindowMouseGrab(window, i);
}
