#ifndef CONSTS_H
#define CONSTS_H

#define BITS_TO_BYTE (8)

#ifndef CACHE_SIZE
#define CACHE_SIZE (64)
#endif

#define CLZ_SENTINEL (0xff_ff_ff_ff_ff_ff_ff_ff)

#define CACHE_SIZE_BITS (CACHE_SIZE * BITS_TO_BYTE)

#endif
