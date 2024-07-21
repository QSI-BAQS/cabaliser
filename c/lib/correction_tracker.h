#ifndef CORRECTION_TRACKER_H
#define CORRECTION_TRACKER_H

#include <stddef.h>
#include "array_list.h" 

struct correction_tracker_t {
    size_t n_qubits; 
    array_list_t* qubits; 
};
typedef struct correction_tracker_t correction_tracker_t;

#endif
