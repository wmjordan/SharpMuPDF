#ifndef JBIG2ENC_JBIG2_LOSSLESS_H__
#define JBIG2ENC_JBIG2_LOSSLESS_H__

#include <stdlib.h>
#include <stdint.h>
#define u64 uint64_t
#define u32 uint32_t
#define u16 uint16_t
#define u8  uint8_t

u8* jbig2_lossless_encode(int width, int height, int stride, bool zeroIsWhite, u8* const source, int* const length);


#endif