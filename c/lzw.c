#pragma once // https://github.com/eloj/lzw-eddy/blob/master/lzw.h
#include "c/lzw.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef uint64_t lzw_node;
typedef uint32_t bitres_t;
typedef uint16_t code_t;
typedef uint8_t sym_t;

struct lzw_string_table {
  uint32_t code_width;
  code_t next_code;
  code_t prev_code;
  lzw_node node[1ull << 16]; // 16K at 12-bit codes.
};

struct lzw_state {
  struct lzw_string_table tree;
  bool was_init;
  bool must_reset;
  size_t rptr;
  size_t wptr;
  bitres_t bitres;
  uint32_t bitres_len;
  size_t longest_prefix;
  size_t longest_prefix_allowed;
};

#define SYMBOL_BITS     8
#define SYMBOL_MASK     ((1UL << SYMBOL_BITS) - 1)
#define PARENT_BITS     16
#define PARENT_SHIFT    SYMBOL_BITS
#define PARENT_MASK     ((1UL << PARENT_BITS) - 1)
#define PREFIXLEN_BITS  16
#define PREFIXLEN_SHIFT (PARENT_BITS + SYMBOL_BITS)
#define PREFIXLEN_MASK  ((1UL << PREFIXLEN_BITS) - 1)

#define CODE_CLEAR (1UL << SYMBOL_BITS)
#define CODE_EOF   (CODE_CLEAR + 1)
#define CODE_FIRST (CODE_CLEAR + 2)

_Static_assert(SYMBOL_BITS <= sizeof(sym_t) * 8, "sym_t type too small");
_Static_assert((SYMBOL_BITS + PARENT_BITS + PREFIXLEN_BITS) <=
                   sizeof(lzw_node) * 8,
               "lzw_node type too small");
_Static_assert((16 * 2 - 1) < sizeof(bitres_t) * 8, "bitres_t type too small");

static inline code_t lzw_node_prefix_len(lzw_node node) {
  return (node >> PREFIXLEN_SHIFT) & PREFIXLEN_MASK;
}

static inline lzw_node lzw_make_node(sym_t symbol, code_t parent, code_t len) {
  lzw_node node = (len << PREFIXLEN_SHIFT) | (parent << PARENT_SHIFT) | symbol;
  return node;
}

static inline uint32_t mask_from_width(uint32_t width) {
  return (1UL << width) - 1;
}

static void lzw_reset(struct lzw_state *state) {
  state->tree.prev_code = CODE_EOF;
  state->tree.next_code = CODE_FIRST;
  state->tree.code_width = 9;
  state->must_reset = false;
}

static void lzw_init(struct lzw_state *state) {
  for (size_t i = 0; i < (1UL << SYMBOL_BITS); ++i)
    state->tree.node[i] = lzw_make_node((sym_t)i, 0, 0);
  state->rptr = 0;
  state->bitres = 0;
  state->bitres_len = 0;
  state->was_init = true;
  lzw_reset(state);
}

int64_t lzw_decompress(const uint8_t *src, size_t slen, uint8_t *dest,
                       size_t dlen) {
  static struct lzw_state _state, *state = &_state;
  if (!state->was_init)
    lzw_init(state);

  // Keep local copies so that we can exit and continue without losing bits.
  uint32_t bitres = state->bitres;
  uint32_t bitres_len = state->bitres_len;

  uint32_t code = 0;
  size_t wptr = 0;

  while (state->rptr < slen) {
    // Fill bit-reservoir.
    while ((bitres_len < state->tree.code_width) && (state->rptr < slen)) {
      bitres |= src[state->rptr++] << bitres_len;
      bitres_len += 8;
    }

    state->bitres = bitres;
    state->bitres_len = bitres_len;

    code = bitres & mask_from_width(state->tree.code_width);
    bitres >>= state->tree.code_width;
    bitres_len -= state->tree.code_width;

    if (code == CODE_CLEAR) {
      if (state->tree.next_code != CODE_FIRST)
        lzw_reset(state);
      continue;
    } else if (code == CODE_EOF) {
      break;
    }

    bool known_code = code < state->tree.next_code;
    code_t tcode = known_code ? code : state->tree.prev_code;
    size_t prefix_len = 1 + lzw_node_prefix_len(state->tree.node[tcode]);
    uint8_t symbol = 0;

    // Track longest prefix seen.
    if (prefix_len > state->longest_prefix) {
      state->longest_prefix = prefix_len;
    }

    // Check if room in output buffer, else return early.
    if (wptr + prefix_len + !known_code > dlen) {
      return wptr;
    }

    // Write out prefix to destination
    for (size_t i = 0; i < prefix_len; ++i) {
      symbol = state->tree.node[tcode] & SYMBOL_MASK;
      dest[wptr + prefix_len - 1 - i] = symbol;
      tcode = (state->tree.node[tcode] >> PARENT_SHIFT) & PARENT_MASK;
    }
    wptr += prefix_len;

    // Add the first character of the prefix as a new code with prev_code as
    // the parent.
    if (state->tree.prev_code != CODE_EOF) {
      if (!known_code) {
        dest[wptr++] = symbol; // Special case for new codes.
      }

      state->tree.node[state->tree.next_code] = lzw_make_node(
          symbol, state->tree.prev_code,
          1 + lzw_node_prefix_len(state->tree.node[state->tree.prev_code]));

      // TODO: Change to ==
      if (state->tree.next_code >= mask_from_width(state->tree.code_width)) {
        if (state->tree.code_width == 16) {
          // Out of bits in code, next code MUST be a reset!
          state->must_reset = true;
          state->tree.prev_code = code;
          continue;
        }
        ++state->tree.code_width;
      }
      state->tree.next_code++;
    }
    state->tree.prev_code = code;
  }
  return wptr;
}
