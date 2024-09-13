#pragma once

#include <stdint.h>
#include <stddef.h>

int64_t lzw_decompress(const uint8_t *src, size_t slen, uint8_t *dest, size_t dlen);
