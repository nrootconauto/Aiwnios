/* See https://viewsourcecode.org/snaptoken/kilo/
 *
 **/
#include "c/aiwn_asm.h"
#include "c/aiwn_mem.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void (*kb_cb)(int64_t, void *);
static void *kb_cb_data;
static void SendOut(int64_t c, int64_t sc) {
  SetWriteNP(1);
  if (kb_cb)
    FFI_CALL_TOS_2(kb_cb, c, sc);
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

// http://www.rohitab.com/discuss/topic/39438-keyboard-driver/
static char keys[] = {
    0,   CH_ESC, '1',  '2', '3',  '4', '5', '6', '7', '8', '9', '0', '-',
    '=', '\b',   '\t', 'q', 'w',  'e', 'r', 't', 'y', 'u', 'i', 'o', 'p',
    '[', ']',    '\n', 0,   'a',  's', 'd', 'f', 'g', 'h', 'j', 'k', 'l',
    ';', '\'',   '`',  0,   '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',',
    '.', '/',    0,    '*', 0,    ' ', 0,   0,   0,   0,   0,   0,   0,
    0,   0,      0,    0,   0,    0,   0,   0,   0,   '-', 0,   '5', 0,
    '+', 0,      0,    0,   0,    0,   0,   0,   0,   0,   0,   0};
static int64_t IsShift(char ch) {
  if (isupper(ch))
    return 1;
  return NULL != strchr("~!@#$%^&*()_+{}|:\"<>?", ch);
}
static int64_t K2SC(char ch) {
  int64_t i = 0;
  const char *shift = "~!@#$%^&*()_+{}|:\"<>?";
  const char *unshift = "`1234567890-=[]\\;',./";
  const char *ptr = strchr(shift, ch);
  if (ptr)
    ch = unshift[ptr - shift];
  for (; i != sizeof(keys) / sizeof(*keys); i++) {
    if (keys[i] == ch)
      return i;
  }
  return -1;
}

#if defined(__linux__) || defined(__FreeBSD__) || defined(__APPLE__) ||        \
    defined(__OpenBSD__)
#  include <SDL.h>
#  include <sys/ioctl.h>
#  include <termios.h>
#  include <unistd.h>
static struct termios orig_termios;
static void enableRawMode() {
  struct termios raw;
  tcgetattr(STDIN_FILENO, &raw);
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag |= (CS8);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}
static void disableRawMode() {
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
  puts("\e[?1006;1015;1003l"); // Disable mouse trackin'
  puts("\e[?1049l");           // Alternate screen buffer
}
static int ReadChr() {
  char c;
  if (1 == read(STDIN_FILENO, &c, 1))
    return c;
  return 0;
}

static void (*ms_cb)(int64_t, int64_t, int64_t, int64_t);
static int64_t ms_z = 0;
void TermSetMsCb(void *c) {
  ms_cb = c;
  // Enable mouse trackin.
  puts("\e[?1003;1006;1016h");
  puts("\e[?1049h"); // Alternate screen buffer
}

static int InputThread(void *ul) {
  char c;
  int64_t flags, lr = 0;
  char _buf[256], *buf = _buf;
  int a, b, c2, ptr;
  int ms;
  while (!*(uint64_t *)ul) {
    flags = 0;
    c = ReadChr();
    if (c == 127) {
      SendOut(CH_BACKSPACE, K2SC(CH_BACKSPACE));
      continue;
    }
    if (c == '\r') {
      SendOut('\n', K2SC('\n'));
      continue;
    }
    if (1 <= c && c <= 26) {
      c = 'a' + c - 1;
      flags = SCF_CTRL;
      SendOut((CH_CTRLA + c - 1), K2SC(c) | flags);
      continue;
    }
    if (c == 27) {
      c = ReadChr();
      if (c == '[') {
        // https://en.wikipedia.org/wiki/ANSI_escape_code
        for (ptr = 0; ptr < 255; ptr++) {
          c = ReadChr();
          if (!c)
            break;
          buf[ptr] = c;
          if (c == '~' || isalpha(c)) {
            ptr++;
            break;
          }
        }
        buf[ptr] = 0;
        a = 1;
        b = 0;
        ms = 0;
        if (buf[0] == '<')
          ms = 1;
        if (strlen(buf + ms) == 1) {
          // ESC[letter
          c = buf[0];
        } else if (ms && strchr(buf + ms, ';')) {
          // Check for a;b;c2c
          int cnt = 0;
          for (ptr = 0; buf[ptr]; ptr++)
            if (buf[ptr] == ';')
              cnt++;
          if (cnt == 1)
            sscanf(buf + ms, "%d;%d%c", &b, &a, &c);
          else if (cnt == 2)
            sscanf(buf + ms, "%d;%d;%d%c", &b, &a, &c2, &c);
        } else {
          sscanf(buf + ms, "%d%c", &a, &c);
        }
        if (b & 1)
          flags |= SCF_SHIFT;
        if (b & 2)
          flags |= SCF_ALT;
        if (b & 4)
          flags |= SCF_CTRL;
        if (ms) {
          if (ms_cb) {
            SetWriteNP(1);
            // https://stackoverflow.com/questions/5966903/how-to-get-mousemove-and-mouseclick-in-bash
            if (c == 'M') {
              a--;          // 0 based instead of 1 based
              c2--;         // ditto 21
              if (b == 2) { // Right click
                lr |= 0b01;
              } else if (b == 0) { // Left click
                lr |= 0b10;
              } else if (b == 64) {
                ms_z--;
              } else if (b == 65) {
                ms_z++;
              }
              FFI_CALL_TOS_4(ms_cb, a * 8 + 4, c2 * 8 + 4, ms_z, lr);
            } else if (c == 'm') {
              a--;
              c2--;
              if (b == 2)
                lr &= ~0b01;
              if (b == 0)
                lr &= ~0b10;
              FFI_CALL_TOS_4(ms_cb, a * 8 + 4, c2 * 8 + 4, ms_z, lr);
            }
          }
          continue;
        }
        switch (c) {
        case 'A':
          SendOut(0, SC_CURSOR_UP | flags);
          break;
        case 'B':
          SendOut(0, SC_CURSOR_DOWN | flags);
          break;
        case 'C':
          SendOut(0, SC_CURSOR_RIGHT | flags);
          break;
        case 'D':
          SendOut(0, SC_CURSOR_LEFT | flags);
          break;
        case 'F':
        end:
          SendOut(0, SC_END | flags);
          break;
        case 'H':
        home:
          SendOut(0, SC_HOME | flags);
          break;
        case 'P':
        f1:
          if (a == 1)
            SendOut(0, SC_F1 | flags);
          break;
        case 'Q':
        f2:
          if (a == 1)
            SendOut(0, SC_F2 | flags);
          break;
        case 'R':
        f3:
          if (a == 1)
            SendOut(0, SC_F3 | flags);
          break;
        case 'S':
        f4:
          if (a == 1)
            SendOut(0, SC_F4 | flags);
          break;
        case '~':
          switch (a) {
          case 1:
            goto home;
          case 2:
            SendOut(0, SC_INS | flags);
            break;
          case 3:
            SendOut(0, SC_DELETE | flags);
            break;
          case 4:
            goto end;
          case 5:
            SendOut(0, SC_PAGE_UP | flags);
            break;
          case 6:
            SendOut(0, SC_PAGE_DOWN | flags);
            break;
          case 7:
            goto home;
          case 8:
            goto end;
          case 10 ... 15:
            SendOut(0, (SC_F1 + a - 10) | flags);
            break;
          case 17 ... 21:
            SendOut(0, (SC_F6 + a - 17) | flags);
            break;
          }
          break;
        }
      } else if (!c)
        SendOut(CH_ESC, SC_ESC);
      else if (c) {
        if (c == 'O') {
          switch (c = ReadChr()) {
          case 'P':
            goto f1;
          case 'Q':
            goto f2;
          case 'R':
            goto f3;
          case 'S':
            goto f4;
          }
        }
        if (1 <= c && c <= 26) {
          c = 'a' + c - 1;
          flags |= SCF_CTRL;
          SendOut(CH_CTRLA + c - 1, SCF_ALT | K2SC(c) | flags);
          continue;
        }
        if (IsShift(c)) {
          flags |= SCF_SHIFT;
          c = tolower(c);
        }
        SendOut(c, SCF_ALT | K2SC(c));
      }
    } else if (c) {
      b = c;
      if (IsShift(c)) {
        flags |= SCF_SHIFT;
        c = tolower(c);
      }
      a = K2SC(c);
      if (a > 0)
        SendOut(b, a | flags);
    }
  }
}
void AiwniosTUIEnable() {
  tcgetattr(STDIN_FILENO, &orig_termios);
  atexit(&disableRawMode);
  enableRawMode();
}
void TUIInputLoop(int64_t *ul) {
  InputThread(ul);
}
void TermSetKbCb(void *fptr) {
  kb_cb = fptr;
}
void TermSize(int64_t *a, int64_t *b) {
  struct winsize wsz;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &wsz);
  if (a)
    *a = wsz.ws_col;
  if (b)
    *b = wsz.ws_row;
}
#else
#  include <windows.h>
#  include <stdio.h>
static DWORD old_stdin_mode, old_stdout_mode;
static HANDLE stdin_handle = NULL;
static HANDLE stdout_handle = NULL;
static int64_t term_x = 20, term_y = 20, ms_x = 0, ms_y = 0, ms_z = 0,
               size_taitned = 0;
static void (*ms_cb)(int64_t, int64_t, int64_t, int64_t);
void TermSize(int64_t *a, int64_t *b) {
  CONSOLE_SCREEN_BUFFER_INFO scrn;
  if (stdout_handle && size_taitned) {
    if (GetConsoleScreenBufferInfo(stdout_handle, &scrn)) {
      term_x = scrn.dwSize.X;
      term_y = scrn.dwSize.Y;
      size_taitned = 0;
    }
  }
  if (a)
    *a = term_x;
  if (b)
    *b = term_y;
}
void TermSetKbCb(void *fptr) {
  kb_cb = fptr;
}
static void AiwniosTUIDisable() {
  SetConsoleMode(stdout_handle, old_stdout_mode);
  SetConsoleMode(stdin_handle, old_stdin_mode);
  CloseHandle(stdin_handle);
  CloseHandle(stdout_handle);
  puts("\e[?1049h"); // Alternate screen buffer
}
void AiwniosTUIEnable() {
  stdin_handle = GetStdHandle(STD_INPUT_HANDLE);
  GetConsoleMode(stdin_handle, &old_stdin_mode);
  SetConsoleMode(stdin_handle, ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT);
  stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
  GetConsoleMode(stdout_handle, &old_stdout_mode);
  SetConsoleMode(stdout_handle,
                 ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
  puts("\e[?1049l"); // Alternate screen buffer
  atexit(&AiwniosTUIDisable);
}
void TermSetMsCb(void *c) {
  ms_cb = c;
}
void TUIInputLoop(int64_t *ul) {
  INPUT_RECORD buf[512];
  DWORD read;
  int64_t i, flags, flags2;
  size_taitned = 1;
  while (!*ul) {
    ReadConsoleInput(stdin_handle, buf, 512, &read);
    for (i = 0; i < read; i++) {
      switch (buf[i].EventType) {
      case KEY_EVENT: {
        KEY_EVENT_RECORD *ptr = &buf[i].Event.KeyEvent;
        flags = 0;
        if (ptr->dwControlKeyState & CAPSLOCK_ON)
          flags |= SCF_CAPS;
        if (ptr->dwControlKeyState & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED))
          flags |= SCF_CTRL;
        if (ptr->dwControlKeyState & (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED))
          flags |= SCF_ALT;
        if (ptr->dwControlKeyState & SHIFT_PRESSED)
          flags |= SCF_SHIFT;

        if (ptr->bKeyDown) {
          switch (ptr->wVirtualKeyCode) {
            break;
          case VK_BACK:
            SendOut(CH_BACKSPACE, SC_BACKSPACE | flags);
            break;
          case VK_TAB:
            SendOut('\t', K2SC('\t') | flags);
            break;
          case VK_RETURN:
            SendOut('\n', K2SC('\n') | flags);
            break;
          case VK_ESCAPE:
            SendOut(0, SC_ESC | flags);
            break;
          case VK_SPACE:
            SendOut(' ', K2SC(' ') | flags);
            break;
          case VK_PRIOR:
            SendOut(0, SC_PAGE_UP | flags);
            break;
          case VK_NEXT:
            SendOut(0, SC_PAGE_DOWN | flags);
            break;
          case VK_END:
            SendOut(0, SC_END | flags);
            break;
          case VK_HOME:
            SendOut(0, SC_HOME | flags);
            break;
          case VK_LEFT:
            SendOut(0, SC_CURSOR_LEFT | flags);
            break;
          case VK_RIGHT:
            SendOut(0, SC_CURSOR_RIGHT | flags);
            break;
          case VK_UP:
            SendOut(0, SC_CURSOR_UP | flags);
            break;
          case VK_DOWN:
            SendOut(0, SC_CURSOR_DOWN | flags);
            break;
          case VK_INSERT:
            SendOut(0, SC_INS | flags);
            break;
          case VK_DELETE:
            SendOut(0, SC_DELETE | flags);
            break;
          case 0x30 ... 0x39:
            SendOut(ptr->uChar.AsciiChar,
                    K2SC(tolower(ptr->uChar.AsciiChar)) | flags);
            break;
          case 0x41 ... 0x5A:
            SendOut(ptr->uChar.AsciiChar,
                    K2SC(tolower(ptr->uChar.AsciiChar)) | flags);
            break;
          case VK_F1 ... VK_F24:
            SendOut(0, (SC_F1 + ptr->wVirtualKeyCode - VK_F1) | flags);
            break;
          case VK_NUMLOCK:
            SendOut(0, SC_NUM | flags);
            break;
          default:
            SendOut(ptr->uChar.AsciiChar,
                    K2SC(tolower(ptr->uChar.AsciiChar)) | flags);
            break;
          }
        }
      } break;
      case WINDOW_BUFFER_SIZE_EVENT: {
        WINDOW_BUFFER_SIZE_RECORD *ptr = &buf[i].Event.WindowBufferSizeEvent;
        term_x = ptr->dwSize.X;
        term_y = ptr->dwSize.Y;
        size_taitned = 0;
      } break;
      case MOUSE_EVENT: {
        MOUSE_EVENT_RECORD *ptr = &buf[i].Event.MouseEvent;
        if (ptr->dwEventFlags & MOUSE_WHEELED)
          if (ptr->dwButtonState >> (8 * sizeof(WORD)) > 0)
            ms_z++;
          else
            ms_z--;
        if (ms_cb) {
          int64_t lr = 0;
          if (ptr->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)
            lr |= 0b10;
          if (ptr->dwButtonState & RIGHTMOST_BUTTON_PRESSED)
            lr |= 0b01;
          FFI_CALL_TOS_4(ms_cb, ptr->dwMousePosition.X * 8 + 4,
                         ptr->dwMousePosition.Y * 8 + 4, ms_z, lr);
        }
      } break;
      }
    }
  }
};
#endif
