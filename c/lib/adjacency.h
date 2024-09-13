#ifndef ADJACENT_H
#define ADJACENT_H

#include <stdint.h>

struct adjacency_obj {
    uint32_t targ; // Target qubit for the adjacencies
    uint32_t n_adjacent; // Number of elements in the array
    uint32_t* adjacencies; // Array of adjacent objects
};
#endif
