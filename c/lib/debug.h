#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>

#ifdef DEBUG

    #define DEBUG_0 0
    #define DEBUG_1 1
    #define DEBUG_2 2
    #define DEBUG_3 3

    #ifndef DEBUG_SRC
        extern const int g_debuglevel;
    #endif

    #define DPRINT(level, ...) ((level <= g_debuglevel) ? printf(__VA_ARGS__) : 0) 

    #define NULL_CHECK(ptr) assert(NULL != ptr)
    #define DEBUG_CHECK(expr) assert(expr)

#else

    #define DPRINT(...) 
    #define NULL_CHECK(...)
    #define DEBUG_CHECK(...)

#endif


#endif
