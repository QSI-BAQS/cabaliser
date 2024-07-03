#ifndef ALIGNED_CHUNK_H
#define ALIGNED_CHUNK_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>


#include "debug.h"

#ifndef CACHE_SIZE
#define CACHE_SIZE (64)
#endif


#define ROW_MAJOR (1)
#define COL_MAJOR (0)

// Always add one so that we can do offset alignment if the manual alignment fails
#define SLICE_LEN(n_qubits) ((n_qubits / CACHE_SIZE) + (((n_qubits % CACHE_SIZE) == 0) ? 0 : 1)) 


#define CHUNK_OBJ uint64_t
#define CACHE_CHUNKS (CACHE_SIZE / sizeof(CHUNK_OBJ)) 

struct aligned_chunk {
   CHUNK_OBJ components[CACHE_CHUNKS]; 
};

struct tableau_slice {
    struct aligned_chunk* chunks; 
};

typedef struct {
    size_t n_qubits;
    void* chunks; // Pointer to allocated chunks
    struct tableau_slice** slices_x; // Slice representation pointers 
    struct tableau_slice** slices_z; // Slice representation pointers 
    bool orientation;
} tableau_t;


tableau_t* tableau_create(const size_t n_qubits);
void tableau_destroy(tableau_t* tab);

void slice_set_bit(struct tableau_slice* slice, const size_t index, uint8_t value);

#endif 
