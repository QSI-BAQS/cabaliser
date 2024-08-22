#ifndef SIMD_TRANSPOSE_H
#define SIMD_TRANSPOSE_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <assert.h>

#include <emmintrin.h>
#include <smmintrin.h>
#include <immintrin.h>

#include <x86gprintrin.h>


// Tableau elements are guaranteed to be aligned in this fashion
typedef uint8_t uint8_tv __attribute__((aligned(16))); 


void simd_transpose_2x16(uint8_tv** src, uint8_tv** targ);

void chunk_transpose_2x16(uint8_tv** src, uint8_tv** targ);




#endif
