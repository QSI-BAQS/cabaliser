#ifndef TEST_TRANSPOSE_H
#define TEST_TRANSPOSE_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <string.h>
#include <assert.h>

#if defined(__x86_64__)
#include <emmintrin.h>
#include <smmintrin.h>
#include <immintrin.h>

#include <x86gprintrin.h>
#endif

// Due to compiler optimisations, some tweaking with flags is needed here.

#endif
