/* See https://viewsourcecode.org/snaptoken/kilo/
 *
 **/
#if defined(__linux__) || defined(__FreeBSD__) || defined(__APPLE__)
#  include "c/aiwn_asm.h"
#  include "c/aiwn_mem.h"
#  include <SDL.h>
#  include <ctype.h>
#  include <stdlib.h>
#  include <string.h>
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
}
static int ReadChr() {
  char c;
  if (1 == read(STDIN_FILENO, &c, 1))
    return c;
  return 0;
}

#  define CH_CTRLA       0x01
#  define CH_CTRLB       0x02
#  define CH_CTRLC       0x03
#  define CH_CTRLD       0x04
#  define CH_CTRLE       0x05
#  define CH_CTRLF       0x06
#  define CH_CTRLG       0x07
#  define CH_CTRLH       0x08
#  define CH_CTRLI       0x09
#  define CH_CTRLJ       0x0A
#  define CH_CTRLK       0x0B
#  define CH_CTRLL       0x0C
#  define CH_CTRLM       0x0D
#  define CH_CTRLN       0x0E
#  define CH_CTRLO       0x0F
#  define CH_CTRLP       0x10
#  define CH_CTRLQ       0x11
#  define CH_CTRLR       0x12
#  define CH_CTRLS       0x13
#  define CH_CTRLT       0x14
#  define CH_CTRLU       0x15
#  define CH_CTRLV       0x16
#  define CH_CTRLW       0x17
#  define CH_CTRLX       0x18
#  define CH_CTRLY       0x19
#  define CH_CTRLZ       0x1A
#  define CH_CURSOR      0x05
#  define CH_BACKSPACE   0x08
#  define CH_ESC         0x1B
#  define CH_SHIFT_ESC   0x1C
#  define CH_SHIFT_SPACE 0x1F
#  define CH_SPACE       0x20

// Scan code flags
#  define SCf_E0_PREFIX 7
#  define SCf_KEY_UP    8
#  define SCf_SHIFT     9
#  define SCf_CTRL      10
#  define SCf_ALT       11
#  define SCf_CAPS      12
#  define SCf_NUM       13
#  define SCf_SCROLL    14
#  define SCf_NEW_KEY   15
#  define SCf_MS_L_DOWN 16
#  define SCf_MS_R_DOWN 17
#  define SCf_DELETE    18
#  define SCf_INS       19
#  define SCf_NO_SHIFT  30
#  define SCf_KEY_DESC  31
#  define SCF_E0_PREFIX (1 << SCf_E0_PREFIX)
#  define SCF_KEY_UP    (1 << SCf_KEY_UP)
#  define SCF_SHIFT     (1 << SCf_SHIFT)
#  define SCF_CTRL      (1 << SCf_CTRL)
#  define SCF_ALT       (1 << SCf_ALT)
#  define SCF_CAPS      (1 << SCf_CAPS)
#  define SCF_NUM       (1 << SCf_NUM)
#  define SCF_SCROLL    (1 << SCf_SCROLL)
#  define SCF_NEW_KEY   (1 << SCf_NEW_KEY)
#  define SCF_MS_L_DOWN (1 << SCf_MS_L_DOWN)
#  define SCF_MS_R_DOWN (1 << SCf_MS_R_DOWN)
#  define SCF_DELETE    (1 << SCf_DELETE)
#  define SCF_INS       (1 << SCf_INS)
#  define SCF_NO_SHIFT  (1 << SCf_NO_SHIFT)
#  define SCF_KEY_DESC  (1 << SCf_KEY_DESC)

// TempleOS places a 1 in bit 7 for
// keys with an E0 prefix.
// See \dLK,"::/Doc/CharOverview.DD"\d and \dLK,"KbdHndlr",A="MN:KbdHndlr"\d().
#  define SC_ESC          0x01
#  define SC_BACKSPACE    0x0E
#  define SC_TAB          0x0F
#  define SC_ENTER        0x1C
#  define SC_SHIFT        0x2A
#  define SC_CTRL         0x1D
#  define SC_ALT          0x38
#  define SC_CAPS         0x3A
#  define SC_NUM          0x45
#  define SC_SCROLL       0x46
#  define SC_CURSOR_UP    0x48
#  define SC_CURSOR_DOWN  0x50
#  define SC_CURSOR_LEFT  0x4B
#  define SC_CURSOR_RIGHT 0x4D
#  define SC_PAGE_UP      0x49
#  define SC_PAGE_DOWN    0x51
#  define SC_HOME         0x47
#  define SC_END          0x4F
#  define SC_INS          0x52
#  define SC_DELETE       0x53
#  define SC_F1           0x3B
#  define SC_F2           0x3C
#  define SC_F3           0x3D
#  define SC_F4           0x3E
#  define SC_F5           0x3F
#  define SC_F6           0x40
#  define SC_F7           0x41
#  define SC_F8           0x42
#  define SC_F9           0x43
#  define SC_F10          0x44
#  define SC_F11          0x57
#  define SC_F12          0x58
#  define SC_PAUSE        0x61
#  define SC_GUI          0xDB
#  define SC_PRTSCRN1     0xAA
#  define SC_PRTSCRN2     0xB7

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
	if(isupper(ch)) return 1;
	return NULL!=strchr("~!@#$%^&*()_+{}|:\"<>?",ch);
}
static int64_t K2SC(char ch) {
  int64_t i = 0;
  const char *shift="~!@#$%^&*()_+{}|:\"<>?";
  const char *unshift="`1234567890-=[]\\;',./";
  const char *ptr=strchr(shift,ch);
  if(ptr) 
    ch=unshift[ptr-shift];
  for (; i != sizeof(keys) / sizeof(*keys); i++) {
    if (keys[i] == ch)
      return i;
  }
  return -1;
}
static void (*kb_cb)(int64_t, void *);
static void *kb_cb_data;
static void SendOut(int64_t c, int64_t sc) {
  SetWriteNP(1);
  if (kb_cb)
    FFI_CALL_TOS_2(kb_cb, c, sc);
}

static int InputThread(void *fptr) {
  char c;
  int64_t flags;
  char buf[256];
  int a, b, ptr;
  while (1) {
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
          if (c == '~' || isalpha(c))
            break;
        }
        buf[ptr] = 0;
        a = 1;
        b = 0;
        if (strlen(buf) == 1) {
          // ESC[letter
          c = buf[0];
        } else if (strchr(buf, ';')) {
          sscanf(buf, "%d;%d%c", &b, &a, &c);
        } else {
          sscanf(buf, "%d%c", &a, &c);
        }
        if (b & 1)
          flags |= SCF_SHIFT;
        if (b & 2)
          flags |= SCF_ALT;
        if (b & 4)
          flags |= SCF_CTRL;
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
            SendOut(0, (SC_F1 + c - 10) | flags);
            break;
          case 17 ... 21:
            SendOut(0, (SC_F6 + c - 10) | flags);
            break;
          }
          break;
        }
      } else if (!c)
        SendOut(CH_ESC, SC_ESC);
      else if (c) {
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
  SDL_CreateThread(&InputThread, "TemrinalInput", NULL);
}
void TermSetKbCb(void *fptr) {
  kb_cb = fptr;
}

#else
void TermSetKbCb(void *fptr) {
  abort();
}
void AiwniosTUIEnable() {
  abort();
}
#endif
