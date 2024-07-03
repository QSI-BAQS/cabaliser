#ifndef DEBUG_H
#define DEBUG_H


#include <stdio.h>

#ifdef DEBUG

#ifndef DEBUG_SRC
extern const int g_debuglevel;
#endif

#define DPRINT(level, ...) ((level <= g_debuglevel) ? printf(__VA_ARGS__) : 0) 
#else
#define DPRINT(...) 
#endif

#define DEBUG_0 0
#define DEBUG_1 1
#define DEBUG_2 2
#define DEBUG_3 3

#endif
