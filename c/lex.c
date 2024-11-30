#include "aiwn_asm.h"
#include "aiwn_except.h"
#include "aiwn_fs.h"
#include "aiwn_lexparser.h"
#include "aiwn_mem.h"
#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
char *LexSrcLink(CLexer *lex, void *task) {
  char buf[STR_LEN];
  if (!lex->file)
    return NULL;
  sprintf(buf, "%s:%d:%d", lex->file->filename, lex->file->ln + 1,
          lex->file->col + 1);
  return A_STRDUP(buf, task);
}

int64_t LexAdvChr(CLexer *lex) {
  //
  // We want to simulate going back a charactor some times. Say we get a '.'
  // It could mean a '.',a '...' or maybe even a TK_F64.so if one of these
  // fails, we can go back in time and check the next item
  //
  if (lex->flags & LEXF_USE_LAST_CHAR) {
    lex->flags &= ~LEXF_USE_LAST_CHAR;
    return lex->cur_char;
  }
enter:;
  CLexFile *file = lex->file, *last;
  int64_t ret;
  if (!lex->file)
    return 0;
  // We terminate the silly sauces with 0 as per ASCII nul character
  if (ret = lex->file->text[lex->file->pos]) {
    lex->file->pos++;
    if (ret == 5 || ret == '\r') // TempleOS cursor charactor
      goto enter;
    if (ret == '\n') {
      lex->file->ln++, lex->file->col = 0;
    } else
      lex->file->col++;
    return lex->cur_char = ret;
  } else {
    // Free all the assets of the previous file
    last = file->last;
    A_FREE(file->text);
    A_FREE(file->dir);
    A_FREE(file->filename);
    A_FREE(file);
    //
    // last points to the last file we were in
    // /*Contexts if file.HC */
    // #include "a.HC" // lex->file->next is NULL
    //
    // /*Contexts of a.HC*/
    // "I like toads"; // lex->file->next is "file.HC"'s CLexFile
    lex->file = last;
    goto enter;
  }
}
static void LexErr(CLexer *lex, char *fmt, ...) {
  va_list lst;
  char buffer[STR_LEN];
  va_start(lst, fmt);
  vsprintf(buffer, fmt, lst);
  va_end(lst);
  if (lex->file) {
    fprintf(AIWNIOS_OSTREAM, "ERR %s:%d:%d %s", lex->file->filename,
            lex->file->ln + 1, lex->file->col + 1, buffer);
  } else {
    fprintf(AIWNIOS_OSTREAM, "ERR ???:?:? %s", buffer);
  }
  throw(*(int64_t *)"Lex\0\0\0\0\0");
}

static void LexWarn(CLexer *lex, char *fmt, ...) {
  va_list lst;
  char buffer[STR_LEN];
  va_start(lst, fmt);
  vsprintf(buffer, fmt, lst);
  va_end(lst);
  if (lex->file) {
    fprintf(AIWNIOS_OSTREAM, "WARN %s:%d:%d %s\n", lex->file->filename,
            lex->file->ln + 1, lex->file->col + 1, buffer);
  } else {
    fprintf(AIWNIOS_OSTREAM, "WARN ???:?:? %s\n", buffer);
  }
}

static int64_t __IsCondDirective(const char *str) {
  if (!strcmp("if", str))
    return 1;
  if (!strcmp("ifaot", str))
    return 1;
  if (!strcmp("ifjit", str))
    return 1;
  if (!strcmp("ifdef", str))
    return 1;
  if (!strcmp("ifndef", str))
    return 1;
  return 0;
}

static int64_t LexSkipTillNewLine(CLexer *lex) {
  int64_t chr;
  while (chr = LexAdvChr(lex)) {
    if (chr == ERR)
      return ERR;
    if (chr == '\n')
      return 0;
  }
  return 0;
}

static int64_t LexSkipMultlineComment(CLexer *lex) {
  int64_t chr;
  while (chr = LexAdvChr(lex)) {
    if (chr == ERR)
      return ERR;
    else if (chr == '*') {
      if ('/' == LexAdvChr(lex))
        return 0;
      else
        lex->flags |= LEXF_USE_LAST_CHAR;
    } else if (chr == '/') {
      if ('*' == LexAdvChr(lex)) {
        if (ERR == LexSkipMultlineComment(lex))
          return ERR;
      } else
        lex->flags |= LEXF_USE_LAST_CHAR;
    }
  }
  LexErr(lex, "Expected end of multi-line comment");
  return ERR;
}

static int64_t LexString(CLexer *lex, int64_t till) {
  int64_t idx, chr1, hex;
  for (idx = 0;;) {
    switch (chr1 = LexAdvChr(lex)) {
    case '\\':
      switch (chr1 = LexAdvChr(lex)) {
        break;
      case 'd':
        chr1 = '$';
        goto ins;
        break;
      case 'n':
        chr1 = '\n';
        goto ins;
        break;
      case '0':
        chr1 = '\0';
        goto ins;
        break;
      case 't':
        chr1 = '\t';
        goto ins;
        break;
      case 'r':
        chr1 = '\r';
        goto ins;
        break;
      case '\\':
        chr1 = '\\';
        goto ins;
        break;
      case '\'':
        chr1 = '\'';
        goto ins;
        break;
      case '\"':
        chr1 = '"';
        goto ins;
        break;
      case 'x':
        hex = 0;
        while (isxdigit(chr1 = LexAdvChr(lex))) {
          hex *= 16;
          switch (chr1) {
          case '0' ... '9':
            hex += chr1 - '0';
          case 'A' ... 'F':
            hex += chr1 - 'A' + 10;
          case 'a' ... 'f':
            hex += chr1 - 'a' + 10;
          }
        }
        lex->flags |= LEXF_USE_LAST_CHAR;
        chr1 = hex;
        goto ins;
        break;
      default:
        LexWarn(lex, "Invalid escape charactor '%c'.", chr1);
        goto ins;
      }
      break;
    default:
    ins:
      if (idx + 1 >= STR_LEN) {
        LexErr(lex, "String exceedes STR_LEN chars.");
        return ERR;
      }
      lex->string[idx++] = chr1;
      break;
    case '\'':
    case '"':
      if (till == chr1)
        goto fin;
      goto ins;
    }
  }
fin:
  lex->str_len = idx;
  lex->string[idx++] = 0;
  return 0;
}

static int64_t LexInt(CLexer *lex, int64_t base) {
  int64_t chr1;
  int64_t ret = 0, digit;
  while (chr1 = LexAdvChr(lex)) {
    switch (chr1) {
      break;
    case '0' ... '9':
      digit = chr1 - '0';
      break;
    case 'A' ... 'F':
      digit = chr1 - 'A' + 10;
      break;
    case 'a' ... 'f':
      digit = chr1 - 'a' + 10;
      break;
    default:
    ret:
      //
      // We consumed the current charactor,which may not be useful now,
      // but it may be useful latter.
      //
      lex->flags |= LEXF_USE_LAST_CHAR;
      return ret;
    }
    if (digit >= base) {
      goto ret;
    }
    // This when repeated "add"s an empty digit,as each digit is a multipl of
    // base
    ret *= base;
    ret += digit;
  }
  return ret;
}
int64_t Lex(CLexer *lex) {
re_enter:;
  int64_t chr1 = LexAdvChr(lex), chr2;
  int64_t has_base = 0, base = 10, integer = 0, decimal = 0, exponet = 0,
          zeros = 0, idx = 0, old_flags, in_else;
  FILE *f;
  char macro_name[STR_LEN];
  CHashDefineStr *define;
  CLexFile *new_file, *actual_file;
  char *dir;
  switch (chr1) {
  case '`':
    return lex->cur_tok = '`';
    break;
  case ';':
    return lex->cur_tok = ';';
    break;
  case ':':
    return lex->cur_tok = ':';
    break;
  case ',':
    return lex->cur_tok = ',';
    break;
  case ' ':
  case '\t':
  case '\n':
    if ((lex->flags & LEXF_UNTIL_NEWLINE) && chr1 == '\n') {
      lex->flags &= ~LEXF_UNTIL_NEWLINE;
      return lex->cur_tok = '\n';
    }
    goto re_enter;
    break;
  case '0' ... '9':
    if (chr1 == '0') {
      chr2 = LexAdvChr(lex);
      switch (chr2) {
        break;
      case 'x':
      case 'X':
        chr1 = LexAdvChr(lex);
        base = 16;
        lex->flags |= LEXF_USE_LAST_CHAR;
        break;
      case 'b':
        chr1 = LexAdvChr(lex);
        base = 2;
        lex->flags |= LEXF_USE_LAST_CHAR;
        break;
      case '0' ... '9':
        lex->flags |= LEXF_USE_LAST_CHAR;
        break;
      default:;
        lex->flags |= LEXF_USE_LAST_CHAR;
        // LexErr(lex,"Unexpected charactor after '0'.");
        // return lex->cur_tok=ERR;
      }
      integer = LexInt(lex, base);
      if (integer == ERR && (lex->flags & LEXF_ERROR))
        return lex->cur_tok = ERR;
      switch (LexAdvChr(lex)) {
        break;
      case 'E':
      case 'e':
        decimal = 0;
        if (base != 10) {
          LexErr(lex, "Expected a base 10 number for floating points.");
          return lex->cur_tok = ERR;
        }
      exponet:
        exponet = 0;
        base = 1; // This holds the multipler for the exponet
        if ('+' == (chr1 = LexAdvChr(lex))) {
        } else if ('-' == chr1) {
          base = -1;
        } else if (isdigit(chr1))
          lex->flags |= LEXF_USE_LAST_CHAR;
        exponet = LexInt(lex, 10);
        exponet *= base;
        lex->flags |= LEXF_USE_LAST_CHAR;
        if (integer == ERR && (lex->flags & LEXF_ERROR)) {
          LexErr(lex, "Expected an exponet.");
          return lex->cur_tok = ERR;
        }
      fin_f64:
        if (decimal) {
          lex->flt =
              integer + decimal * pow(10, -zeros - 1 - floor(log10(decimal)));
        } else
          lex->flt = integer;
        lex->flt *= pow(10, exponet);
        return lex->cur_tok = TK_F64;
        break;
      case '.':
      decimal_chk:
        switch (chr1 = LexAdvChr(lex)) {
          break;
        case '0':
          zeros++;
          goto decimal_chk;
          break;
        case '1' ... '9':
          lex->flags |= LEXF_USE_LAST_CHAR;
          decimal = LexInt(lex, 10);
          lex->flags |= LEXF_USE_LAST_CHAR;
          break;
        default:;
        }
        if (lex->cur_char == 'e' || lex->cur_char == 'E') {
          lex->flags &= ~LEXF_USE_LAST_CHAR;
          goto exponet;
        }
        lex->flags |= LEXF_USE_LAST_CHAR;
        lex->integer = integer;
        goto fin_f64;
        break;
      default:
        lex->flags |= LEXF_USE_LAST_CHAR;
        lex->integer = integer;
        return lex->cur_tok = TK_I64;
      }
    } else
      //
      // We checked the first digit and checked for 0.
      // Use it again in LexInt as we found no use for it here.
      //
      lex->flags |= LEXF_USE_LAST_CHAR;
    integer = LexInt(lex, base);
    lex->flags |= LEXF_USE_LAST_CHAR;
    if (integer == ERR && (lex->flags & LEXF_ERROR))
      return lex->cur_tok = ERR;
    lex->integer = integer;
    if (lex->cur_char == '.') {
      lex->flags &= ~LEXF_USE_LAST_CHAR;
      goto decimal_chk;
    }
    if (lex->cur_char == 'e' || lex->cur_char == 'E') {
      lex->flags &= ~LEXF_USE_LAST_CHAR;
      goto exponet;
    }
    return lex->cur_tok = TK_I64;
    break;
  case '+':
    switch (chr1 = LexAdvChr(lex)) {
      break;
    case '+':
      return lex->cur_tok = TK_INC_INC;
      break;
    case '=':
      return lex->cur_tok = TK_ADD_EQ;
      break;
    default:
      lex->flags |= LEXF_USE_LAST_CHAR;
      return lex->cur_tok = '+';
    }
    break;
  case '-':
    switch (chr1 = LexAdvChr(lex)) {
      break;
    case '-':
      return lex->cur_tok = TK_DEC_DEC;
      break;
    case '>':
      return lex->cur_tok = TK_ARROW;
      break;
    case '=':
      return lex->cur_tok = TK_SUB_EQ;
      break;
    default:
      lex->flags |= LEXF_USE_LAST_CHAR;
      return lex->cur_tok = '-';
    }
    break;
  case '=':
    if ('=' == LexAdvChr(lex))
      return lex->cur_tok = TK_EQ_EQ;
    lex->flags |= LEXF_USE_LAST_CHAR;
    return lex->cur_tok = '=';
    break;
  case '(':
    return lex->cur_tok = '(';
    break;
  case ')':
    return lex->cur_tok = ')';
    break;
  case '{':
    return lex->cur_tok = '{';
    break;
  case '}':
    return lex->cur_tok = '}';
    break;
  case '[':
    return lex->cur_tok = '[';
    break;
  case ']':
    return lex->cur_tok = ']';
    break;
  case '.':
    switch (chr1 = LexAdvChr(lex)) {
    case '.':
      if ('.' != LexAdvChr(lex)) {
        LexErr(lex, "Expected 3 dots.");
        return ERR;
      }
      return lex->cur_tok = TK_DOT_DOT_DOT;
      break;
    default:
      lex->flags |= LEXF_USE_LAST_CHAR;
      return lex->cur_tok = '.';
    }
    break;
  case '!':
    if ('=' == LexAdvChr(lex))
      return lex->cur_tok = TK_NE;
    lex->flags |= LEXF_USE_LAST_CHAR;
    return lex->cur_tok = '!';
    break;
  case '~':
    return lex->cur_tok = '~';
    break;
  case '*':
    if ('=' == LexAdvChr(lex))
      return lex->cur_tok = TK_MUL_EQ;
    lex->flags |= LEXF_USE_LAST_CHAR;
    return lex->cur_tok = '*';
    break;
  case '/':
    if ('=' == (chr1 = LexAdvChr(lex)))
      return lex->cur_tok = TK_DIV_EQ;
    else if (chr1 == '/') {
      LexSkipTillNewLine(lex);
      goto re_enter;
    } else if (chr1 == '*') {
      LexSkipMultlineComment(lex);
      goto re_enter;
    }
    lex->flags |= LEXF_USE_LAST_CHAR;
    return lex->cur_tok = '/';
    break;
  case '%':
    if ('=' == LexAdvChr(lex))
      return lex->cur_tok = TK_MOD_EQ;
    lex->flags |= LEXF_USE_LAST_CHAR;
    return lex->cur_tok = '%';
    break;
  case '<':
    switch (chr1 = LexAdvChr(lex)) {
      break;
    case '=':
      return lex->cur_tok = TK_LE;
      break;
    case '<':
      if ('=' == LexAdvChr(lex))
        return lex->cur_tok = TK_LSH_EQ;
      lex->flags |= LEXF_USE_LAST_CHAR;
      return lex->cur_tok = TK_LSH;
      break;
    default:
      lex->flags |= LEXF_USE_LAST_CHAR;
      return lex->cur_tok = '<';
    }
    break;
  case '>':
    switch (chr1 = LexAdvChr(lex)) {
      break;
    case '=':
      return lex->cur_tok = TK_GE;
      break;
    case '>':
      if ('=' == LexAdvChr(lex))
        return lex->cur_tok = TK_RSH_EQ;
      lex->flags |= LEXF_USE_LAST_CHAR;
      return lex->cur_tok = TK_RSH;
      break;
    default:
      lex->flags |= LEXF_USE_LAST_CHAR;
      return lex->cur_tok = '>';
    }
    break;
  case '&':
    switch (LexAdvChr(lex)) {
    case '=':
      return lex->cur_tok = TK_AND_EQ;
    case '&':
      return lex->cur_tok = TK_AND_AND;
    }
    lex->flags |= LEXF_USE_LAST_CHAR;
    return lex->cur_tok = '&';
    break;
  case '|':
    switch (LexAdvChr(lex)) {
    case '=':
      return lex->cur_tok = TK_OR_EQ;
    case '|':
      return lex->cur_tok = TK_OR_OR;
    }
    lex->flags |= LEXF_USE_LAST_CHAR;
    return lex->cur_tok = '|';
    break;
  case '^':
    switch (LexAdvChr(lex)) {
    case '=':
      return lex->cur_tok = TK_XOR_EQ;
    case '^':
      return lex->cur_tok = TK_XOR_XOR;
    }
    lex->flags |= LEXF_USE_LAST_CHAR;
    return lex->cur_tok = '^';
    break;
  case '"':
    if (ERR == LexString(lex, '"'))
      return lex->cur_tok = ERR;
    return lex->cur_tok = TK_STR;
    break;
  case '\'':
    memset(lex->string, 0, 8);
    if (ERR == LexString(lex, '\''))
      return ERR;
    if (strlen(lex->string) > 8) {
      LexErr(lex, "String constant too long!");
      return ERR;
    }
    lex->integer = *(uint64_t *)lex->string;
    return lex->cur_tok = TK_CHR;
    break;
  case '_':
  case '@':
  case 'a' ... 'z':
  case 'A' ... 'Z':
    lex->flags |= LEXF_USE_LAST_CHAR;
    for (;;) {
      switch (chr1 = LexAdvChr(lex)) {
        break;
      case '_':
      case '@':
      case 'a' ... 'z':
      case 'A' ... 'Z':
      case '0' ... '9':
        if (idx + 1 >= STR_LEN) {
          LexErr(lex, "Name is too long.");
          return lex->cur_tok = ERR;
        }
        lex->string[idx++] = chr1;
        break;
      default:
        lex->flags |= LEXF_USE_LAST_CHAR;
        lex->string[idx++] = 0;
        if (!(lex->flags & LEXF_NO_EXPAND)) {
          define = HashFind(lex->string, Fs->hash_table, HTT_DEFINE_STR, 1);
          if (define) {
            //
            // Move the of cursor backwards as we moved forwards when getting
            // the name NULL+
            // ---->Cur
            //   to
            // --->(at plus)
            //
            lex->file->pos--;
            // We dont want to use the last charactor now that we are in a macro
            lex->flags &= ~LEXF_USE_LAST_CHAR;
            //
            new_file = A_MALLOC(sizeof(CLexFile), NULL);
            new_file->dir = NULL;
            new_file->last = lex->file;
            new_file->filename = A_STRDUP(define->base.str, NULL);
            new_file->pos = new_file->col = new_file->ln = 0;
            new_file->text = A_STRDUP(define->data, NULL);
            lex->file = new_file;
            goto re_enter;
          }
        }
        lex->str_len = idx;
        return lex->cur_tok = TK_NAME;
      }
    }
    break;
  case '#':
    if (TK_NAME == Lex(lex)) {
      if (!strcmp(lex->string, "help_file")) {
        Lex(lex); // Go past string
        LexWarn(lex, "AIWN ignore's #help_file's ignored by the C side");
        goto re_enter;
      } else if (!strcmp(lex->string, "help_index")) {
        Lex(lex); // Go past string
        LexWarn(lex, "AIWN ignore's #help_index's ignored by the C side");
        goto re_enter;
      } else if (!strcmp(lex->string, "assert")) {
        LexWarn(lex, "AIWN ignore's #assert's and other JIT shennangins");
        goto re_enter;
      } else if (!strcmp(lex->string, "if")) {
        lex->flags |= LEXF_UNTIL_NEWLINE;
        CCmpCtrl *cc = CmpCtrlNew(lex);
        char *code;
        int64_t ret = 0;
        CodeCtrlPush(cc);
        Lex(lex);
        if (ParseExpr(cc, 0)) {
          CRPN *rtrn = A_CALLOC(sizeof(CRPN), 0);
          rtrn->type = IC_RET;
          QueIns(rtrn, cc->code_ctrl->ir_code);
          if (code = Compile(cc, NULL, NULL, NULL)) {
            ret = FFI_CALL_TOS_0(code);
          }
          A_FREE(code);
        }
        CodeCtrlPop(cc);
        CmpCtrlDel(cc);
        if (ret)
          goto re_enter;
        goto if_fail;
      } else if (!strcmp(lex->string, "ifaot")) {
        // AIWN will only AOT compile for you,the JIT will be
        // handled by the newly compiled compiler
        goto re_enter;
      } else if (!strcmp(lex->string, "ifjit")) {
        // AIWN will only AOT compile for you,the JIT will be
        // handled by the newly compiled compiler
        goto if_fail;
      } else if (!strcmp(lex->string, "endif")) {
        goto re_enter;
      } else if (!strcmp(lex->string, "else")) {
        in_else = 1;
        // You are here are you passed the initial #if statement?
        // Otherwise you would be searching for an #else
        goto if_fail;
      } else if (!strcmp(lex->string, "ifndef")) {
        old_flags = lex->flags;
        lex->flags |= LEXF_NO_EXPAND;
        if (TK_NAME != Lex(lex)) {
          LexErr(lex, "Expected a name for #ifndef.");
          return lex->cur_tok = ERR;
        }
        lex->flags = old_flags;
        if (HashFind(lex->string, Fs->hash_table, HTT_DEFINE_STR, 1)) {
          in_else = 0;
          goto if_fail;
        } else
          goto if_pass;
      }
      if (!strcmp(lex->string, "ifdef")) {
        old_flags = lex->flags;
        lex->flags |= LEXF_NO_EXPAND;
        if (TK_NAME != Lex(lex)) {
          LexErr(lex, "Expected a name for #ifdef.");
          return lex->cur_tok = ERR;
        }
        lex->flags = old_flags;
        if (!HashFind(lex->string, Fs->hash_table, HTT_DEFINE_STR, 1)) {
          in_else = 0;
        if_fail:
          //
          // idx is the current nesting level,it increases every time
          // we encounter a #ifdef(and it's freinds)
          //
          // #ifdef A // idx==1
          // #ifdef B // idx==2
          // #ifdef C // idx==2
          // #endif // idx==1
          // #endif // idx==0
          //
          idx = 1;
        if_fail_loop:
          while ('#' != (chr1 = LexAdvChr(lex))) {
            if (chr1 == 0)
              return lex->cur_tok = 0;
            if (chr1 == ERR)
              return lex->cur_tok = ERR;
          }
          if (TK_NAME != Lex(lex)) {
            LexErr(lex, "Stray '#' in program.");
            return lex->cur_tok = ERR;
          }
          if (!in_else && !strcmp(lex->string, "else")) {
            goto re_enter;
          } else if (!strcmp(lex->string, "endif")) {
            if (!--idx)
              goto re_enter;
          } else if (__IsCondDirective(lex->string)) {
            idx++;
          }
          goto if_fail_loop;
        } else {
        if_pass:;
        }
        lex->flags = old_flags;
        goto re_enter;
      }
      if (!strcmp(lex->string, "include")) {
        if (TK_STR != Lex(lex)) {
          LexErr(lex, "Expected a string for #include.");
          return lex->cur_tok = ERR;
        }
        actual_file = lex->file;
        while (actual_file) {
          if (actual_file->is_file)
            break;
          actual_file = actual_file->last;
        }
        if (actual_file) {
          if (actual_file->dir) {
            int64_t len =
                snprintf(NULL, 0, "%s/%s", actual_file->dir, lex->string);
            char buf[len + 1];
            sprintf(buf, "%s/%s", actual_file->dir, lex->string);
            dir = __AIWNIOS_StrDup(buf, NULL);
            *strrchr(dir, '/') = 0;
            f = fopen(buf, "rb");
          } else
            goto normal;
        } else {
        normal:
          f = fopen(lex->string, "rb");
          if (!strrchr(lex->string, '/')) {
            dir = __AIWNIOS_StrDup(".", NULL);
          } else {
            dir = __AIWNIOS_StrDup(lex->string, NULL);
            *strrchr(dir, '/') = 0;
          }
        }
        if (!f) {
          LexErr(lex, "I can't find file '%s'.", lex->string);
          return lex->cur_tok = ERR;
        }

        fseek(f, 0, SEEK_END);
        idx = ftell(f);
        fseek(f, 0, SEEK_SET);
        idx -= ftell(f);
        new_file = A_MALLOC(sizeof(CLexFile), NULL);
        new_file->text = A_MALLOC(idx + 1, NULL);
        new_file->text[idx] = 0;
        new_file->dir = dir;
        fread(new_file->text, 1, idx, f);
        fclose(f);
        new_file->is_file = 1;
        new_file->last = lex->file;
        new_file->filename = A_STRDUP(lex->string, NULL);
        new_file->pos = new_file->col = new_file->ln = 0;
        lex->file = new_file;
        goto re_enter;
      } else if (!strcmp(lex->string, "define")) {
        lex->flags |= LEXF_NO_EXPAND;
        if (TK_NAME != Lex(lex)) {
          LexErr(lex, "Expected a name for #define.");
          return lex->cur_tok = ERR;
        }
        lex->flags &= ~LEXF_NO_EXPAND;
        strcpy(macro_name, lex->string);
        strcpy(lex->string, "");
        define = A_CALLOC(sizeof(CHashDefineStr), NULL);
        define->base.str = A_STRDUP(macro_name, NULL);
        define->base.type = HTT_DEFINE_STR;
        define->src_link = LexSrcLink(lex, NULL);
        idx = 0;
        while (chr1 = LexAdvChr(lex)) {
          switch (chr1) {
            break;
          //
          // When we get a '/',it could mean a '//',a '/*',or even just a slash
          // So check for all orf these conditions
          //
          case '/':
            if ('/' == (chr1 = LexAdvChr(lex))) {
              LexSkipTillNewLine(lex);
            add_macro:
              lex->string[idx++] = 0;
              define->data = A_STRDUP(lex->string, NULL);
              HashAdd(&define->base, Fs->hash_table);
              // Lex next item
              goto re_enter;
            } else if (chr1 == '*') {
              LexSkipMultlineComment(lex);
              goto add_macro;
            } else {
              // Is just a '/',but we didnt use the last character so re-use it
              lex->flags |= LEXF_USE_LAST_CHAR;
              lex->string[idx++] = '/';
            }
            break;
          case '\\':
            LexSkipTillNewLine(lex);
            break;
          case '\n':
            lex->string[idx++] = '\n';
            goto add_macro;
            break;
          default:
            lex->string[idx++] = chr1;
          }
        }
      }
    } else {
      LexErr(lex, "Stray '#' in program.");
      return lex->cur_tok = ERR;
    }
    break;
  case 0:
    return lex->cur_tok = 0;
  }
  LexErr(lex, "Unexpected charactor '%c'.", chr1);
  return lex->cur_tok = ERR;
}

CLexer *LexerNew(char *filename, char *text) {
  static int init;
  if (!init) {
    struct {
      char *name;
      int64_t tok;
    } kws[] = {
        //{"include",TK_KW_INCLUDE},
        //{"define",TK_KW_DEFINE},
        {"union", TK_KW_UNION},
        {"catch", TK_KW_CATCH},
        {"class", TK_KW_CLASS},
        {"try", TK_KW_TRY},
        {"if", TK_KW_IF},
        {"else", TK_KW_ELSE},
        {"for", TK_KW_FOR},
        {"while", TK_KW_WHILE},
        {"extern", TK_KW_EXTERN},
        {"_extern", TK_KW__EXTERN},
        {"return", TK_KW_RETURN},
        {"sizeof", TK_KW_SIZEOF},
        {"_intern", TK_KW__INTERN},
        {"do", TK_KW_DO},
        {"asm", TK_KW_ASM},
        {"goto", TK_KW_GOTO},
        //{"exe",TK_KW_EXE},
        {"break", TK_KW_BREAK},
        {"switch", TK_KW_SWITCH},
        {"start", TK_KW_START},
        {"end", TK_KW_END},
        {"case", TK_KW_CASE},
        {"default", TK_KW_DEFUALT},
        {"public", TK_KW_PUBLIC},
        {"offset", TK_KW_OFFSET},
        {"import", TK_KW_IMPORT},
        {"_import", TK_KW__IMPORT},
        //{"ifdef",TK_KW_IFDEF},
        //{"ifndef",TK_KW_IFNDEF},
        //{"ifaot",TK_KW_IFAOT},
        //{"ifjit",TK_KW_IFJIT},
        //{"endif",TK_KW_ENDIF},
        //{"assert",TK_KW_ASSERT},
        {"reg", TK_KW_REG},
        {"noreg", TK_KW_NOREG},
        {"lastclass", TK_KW_LASTCLASS},
        {"no_warn", TK_KW_NOWARN},
        //{"help_index",TK_KW_HELP_INDEX},
        //{"help_file",TK_KW_HELP_FILE},
        {"static", TK_KW_STATIC},
        {"lock", TK_KW_LOCK},
        {"defined", TK_KW_DEFINED},
        {"interrupt", TK_KW_INTERRUPT},
        {"haserrcode", TK_KW_HASERRCODE},
        {"argpop", TK_KW_ARGPOP},
        {"noargpop", TK_KW_NOARGPOP},
    };
    int64_t i;
    CHashKeyword *kw;
    for (i = 0; i != sizeof(kws) / sizeof(*kws); i++) {
      kw = A_CALLOC(sizeof *kw, NULL);
      kw->base.type = HTT_KEYWORD;
      kw->base.str = A_STRDUP(kws[i].name, NULL);
      kw->tk = kws[i].tok;
      HashAdd(kw, Fs->hash_table);
    }
    init = 1;
  }
  CLexer *new = A_CALLOC(sizeof(CLexer), NULL);
  new->hc = HeapCtrlInit(NULL, Fs, 0);
  CLexFile *file = new->file = A_CALLOC(sizeof(CLexFile), new->hc);
  file->filename = A_STRDUP(filename, new->hc);
  file->text = A_STRDUP(text, new->hc);
  return new;
}

void LexerDel(CLexer *lex) {
  HeapCtrlDel(lex->hc);
  A_FREE(lex);
}

void LexerDump(CLexer *lex) {
  int64_t tok;
  while (tok = Lex(lex)) {
    printf("TOK:");
    switch (tok) {
      break;
    case ERR:
      printf("ERR");
      break;
    case TK_I64:
      printf("I64:%ld", lex->integer);
      break;
    case TK_F64:
      printf("F64:%n", lex->flt);
      break;
    case TK_NAME:
      printf("NAME:%s", lex->string);
      break;
    case TK_STR:
      printf("STRING:%s", lex->string);
      break;
    case TK_CHR:
      lex->string[8] = 0;
      printf("CHR:%s", &lex->integer);
      break;
    case TK_ERR:
      printf("ERR");
      return;
      break;
    case TK_INC_INC:
      printf("++");
      break;
    case TK_DEC_DEC:
      printf("--");
      break;
    case TK_AND_AND:
      printf("&&");
      break;
    case TK_XOR_XOR:
      printf("^^");
      break;
    case TK_OR_OR:
      printf("||");
      break;
    case TK_EQ_EQ:
      printf("==");
      break;
    case TK_NE:
      printf("!=");
      break;
    case TK_LE:
      printf("<=");
      break;
    case TK_GE:
      printf(">=");
      break;
    case TK_LSH:
      printf("<<");
      break;
    case TK_RSH:
      printf(">>");
      break;
    case TK_ADD_EQ:
      printf("+=");
      break;
    case TK_SUB_EQ:
      printf("-=");
      break;
    case TK_MUL_EQ:
      printf("*=");
      break;
    case TK_DIV_EQ:
      printf("/=");
      break;
    case TK_LSH_EQ:
      printf("<<=");
      break;
    case TK_RSH_EQ:
      printf(">>=");
      break;
    case TK_AND_EQ:
      printf("&=");
      break;
    case TK_OR_EQ:
      printf("|=");
      break;
    case TK_XOR_EQ:
      printf("^=");
      break;
    case TK_ARROW:
      printf("->");
      break;
    case TK_DOT_DOT_DOT:
      printf("...");
      break;
    case TK_MOD_EQ:
      printf("%=");
      break;
    default:
      if (tok < 0x100)
        printf("%c", tok);
    }
    printf("\n");
  }
}
#ifdef AIWNIOS_TESTS
#  include <assert.h>
void LexTests() {
  char tmpf[BUFSIZ];
  tmpnam(tmpf);
  CLexer *lex = LexerNew("None", "if poodle\n"
                                 "0 123 0b11 0xfeF0\n"
                                 "'abcd1234'\n"
                                 "\"ab\\\"c\"\n"
                                 "++\n"
                                 "--\n"
                                 "&&^^||\n"
                                 "== !=\n"
                                 "<= >= <>\n"
                                 "<<\n"
                                 ">>\n"
                                 "+=\n"
                                 "-=\n"
                                 "*=\n"
                                 "/=\n"
                                 "<<=\n"
                                 ">>=\n"
                                 "&=\n"
                                 "|=\n"
                                 "^=\n"
                                 "->\n"
                                 "...\n"
                                 "%=\n"
                                 "#define onion o \n"
                                 "onion\n"
                                 "#ifdef onion\n"
                                 "1\n"
                                 "#endif\n"
                                 "#ifndef onion\n"
                                 "0\n"
                                 "#endif\n"
                                 "#ifdef onion2\n"
                                 "0\n"
                                 "#endif\n"
                                 "#ifndef onion2\n"
                                 "11\n"
                                 "#endif\n"
                                 "#ifdef onion\n"
                                 "111\n"
                                 "#else\n"
                                 "0\n"
                                 "#endif\n"
                                 "#ifndef onion\n"
                                 "0\n"
                                 "#else\n"
                                 "1111\n"
                                 "#endif\n"
                                 "#ifndef onion2\n"
                                 "11111\n"
                                 "#else\n"
                                 "0\n"
                                 "#endif\n"
                                 // Nested #if test
                                 "#ifdef onion\n"
                                 "#ifdef onion2\n"
                                 "#else\n"
                                 "#ifdef onion2\n"
                                 "#else\n"
                                 "111111\n"
                                 "#endif\n"
                                 "#endif\n"
                                 "#endif\n"
                                 "123.\n"
                                 "123.000456\n"
                                 "123e10\n"
                                 "123e+10\n"
                                 "123e-10\n"
                                 "123.456e+10\n"
                                 "// 123 \n"
                                 "1234\n"
                                 "/*\n"
                                 "1\n"
                                 "2\n"
                                 "*/\n"
                                 "12345\n"
                                 "#define onion 123456 //poop \n"
                                 "onion onion\n"
                                 "#define onion 1234567 /*poop */\n"
                                 "onion onion\n"
                                 "#define onion 1 \\ \n"
                                 "2 \\ \n"
                                 "3 \n"
                                 "onion\n");
  assert(TK_NAME == Lex(lex));
  assert(HashFind("if", Fs->hash_table, HTT_KEYWORD, 1));
  assert(TK_NAME == Lex(lex));
  assert(!strcmp(lex->string, "poodle"));
  assert(TK_I64 == Lex(lex));
  assert(lex->integer == 0);
  assert(TK_I64 == Lex(lex));
  assert(lex->integer == 123);
  assert(TK_I64 == Lex(lex));
  assert(lex->integer == 0b11);
  assert(TK_I64 == Lex(lex));
  assert(lex->integer == 0xfef0);
  assert(TK_CHR == Lex(lex));
  assert(lex->integer == *(int64_t *)"abcd1234");
  assert(TK_STR == Lex(lex));
  assert(!strcmp(lex->string, "ab\"c"));
  assert(TK_INC_INC == Lex(lex));
  assert(TK_DEC_DEC == Lex(lex));
  assert(TK_AND_AND == Lex(lex));
  assert(TK_XOR_XOR == Lex(lex));
  assert(TK_OR_OR == Lex(lex));
  assert(TK_EQ_EQ == Lex(lex));
  assert(TK_NE == Lex(lex));
  assert(TK_LE == Lex(lex));
  assert(TK_GE == Lex(lex));
  assert('<' == Lex(lex));
  assert('>' == Lex(lex));
  assert(TK_LSH == Lex(lex));
  assert(TK_RSH == Lex(lex));
  assert(TK_ADD_EQ == Lex(lex));
  assert(TK_SUB_EQ == Lex(lex));
  assert(TK_MUL_EQ == Lex(lex));
  assert(TK_DIV_EQ == Lex(lex));
  assert(TK_LSH_EQ == Lex(lex));
  assert(TK_RSH_EQ == Lex(lex));
  assert(TK_AND_EQ == Lex(lex));
  assert(TK_OR_EQ == Lex(lex));
  assert(TK_XOR_EQ == Lex(lex));
  assert(TK_ARROW == Lex(lex));
  assert(TK_DOT_DOT_DOT == Lex(lex));
  assert(TK_MOD_EQ == Lex(lex));
  assert(TK_NAME == Lex(lex));
  assert(!strcmp(lex->string, "o"));
  assert(TK_I64 == Lex(lex));
  assert(lex->integer == 1);
  assert(TK_I64 == Lex(lex));
  assert(lex->integer == 11);
  assert(TK_I64 == Lex(lex));
  assert(lex->integer == 111);
  assert(TK_I64 == Lex(lex));
  assert(lex->integer == 1111);
  assert(TK_I64 == Lex(lex));
  assert(lex->integer == 11111);
  assert(TK_I64 == Lex(lex));
  assert(lex->integer == 111111);
#  define AROUND(v, n, off) (((v) >= (n) - (off)) && ((v) <= (n) + (off)))
  assert(TK_F64 == Lex(lex));
  assert(AROUND(lex->flt, 123., 0.1));
  assert(TK_F64 == Lex(lex));
  assert(AROUND(lex->flt, 123.000456, 0.0001));
  assert(TK_F64 == Lex(lex));
  assert(AROUND(lex->flt, 123e10, 1));
  assert(TK_F64 == Lex(lex));
  assert(AROUND(lex->flt, 123e10, 1));
  assert(TK_F64 == Lex(lex));
  assert(AROUND(lex->flt, 123e-10, 1));
  assert(TK_F64 == Lex(lex));
  assert(AROUND(lex->flt, 123.456e10, 1));
  assert(TK_I64 == Lex(lex));
  assert(lex->integer == 1234);
  assert(TK_I64 == Lex(lex));
  assert(lex->integer == 12345);
  assert(TK_I64 == Lex(lex));
  assert(lex->integer == 123456);
  assert(TK_I64 == Lex(lex));
  assert(lex->integer == 123456);
  assert(TK_I64 == Lex(lex));
  assert(lex->integer == 1234567);
  assert(TK_I64 == Lex(lex));
  assert(lex->integer == 1234567);
  assert(TK_I64 == Lex(lex));
  assert(lex->integer == 1);
  assert(TK_I64 == Lex(lex));
  assert(lex->integer == 2);
  assert(TK_I64 == Lex(lex));
  assert(lex->integer == 3);
}
#endif
