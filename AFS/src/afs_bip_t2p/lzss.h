#pragma once

#include <stdint.h>

int decompress_lzss(uint8_t *dst, uint8_t *src, uint32_t srclen);
uint8_t *compress_lzss(uint8_t *dst, uint32_t dstlen, uint8_t *src, uint32_t srcLen);