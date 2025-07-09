#ifndef SIMD_HEADERS_H
#define SIMD_HEADERS_H

#if defined(__x86_64__)
#include <emmintrin.h>
#include <smmintrin.h>
#include <immintrin.h>
#include <x86gprintrin.h>
#define ROWSUM_STRIDE (256 / 8) 
#elif defined(__arm__) || defined(__aarch64__)
#include <arm_neon.h>
#include <arm_sve.h>
#include <arm_neon_sve_bridge.h>
// Halved ROWSUM_STRIDE for Neon
#define ROWSUM_STRIDE (128 / 8) 
#endif


#endif 
