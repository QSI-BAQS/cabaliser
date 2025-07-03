#ifndef SIMD_HEADERS_H
#define SIMD_HEADERS_H

// TODO - these might be gcc only
#if defined(__x86_64__)
#include <emmintrin.h>
#include <smmintrin.h>
#include <immintrin.h>
#include <x86gprintrin.h>
#define ROWSUM_STRIDE (256 / 8) 
#elif defined(__arm__) || defined(__aarch64__)
#include <arm_neon.h>
// The suffix for Neon function calls (must match TABLEAU_SIMD_VEC type)
// u8 is for vectors of uint8s (ie. a uint8x16_t)
#define NEON_SUFFIX u8
// Halved ROWSUM_STRIDE for Neon
#define ROWSUM_STRIDE (128 / 8) 
#endif


#endif 
