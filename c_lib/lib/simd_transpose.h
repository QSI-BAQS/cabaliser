#ifndef SIMD_TRANSPOSE_H
#define SIMD_TRANSPOSE_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <string.h>

#include <assert.h>

#include <emmintrin.h>
#include <smmintrin.h>
#include <immintrin.h>

#include <x86gprintrin.h>

// 16 Byte cache line aligned elements
typedef uint8_t uint8_ta16 __attribute__((aligned(16))); 

// 64 Byte cache line aligned elements
typedef uint64_t uint64_ta64 ;//__attribute__((aligned(32))); 

void simd_transpose_2x16(uint8_t** restrict src, uint8_t** restrict targ);
void simd_transpose_64x64(uint64_t* restrict src[64], uint64_t* restrict targ[64]);
void simd_transpose_64x64_inplace(uint64_t* src[64]);

void chunk_transpose_2x16(uint8_t** restrict src, uint8_t** restrict targ);
void chunk_transpose_64x64(uint64_t* restrict src[64], uint64_t* restrict targ[64]);

void transpose_naive(uint64_t** block, size_t size);

#endif
