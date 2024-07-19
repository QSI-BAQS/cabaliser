#ifndef TABLEAU_H
#define TABLEAU_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include "consts.h"
#include "debug.h"
#include "instructions.h"


#define ROW_MAJOR (1)
#define COL_MAJOR (0)


// Always add one so that we can do offset alignment if the manual alignment fails
#define SLICE_LEN(n_qubits) ((n_qubits / CACHE_SIZE) + (!((n_qubits % CACHE_SIZE) == 0))) 

#define CHUNK_OBJ uint64_t

#define CHUNK_SIZE_BYTES (sizeof(CHUNK_OBJ))
#define CHUNK_SIZE_BITS (CHUNK_SIZE_BYTES * BITS_TO_BYTE)
#define CACHE_CHUNKS (CACHE_SIZE / CHUNK_SIZE_BYTES) 

#define __CHUNK_CTZ __builtin_ctzll 


struct aligned_chunk {
   CHUNK_OBJ components[CACHE_CHUNKS]; 
};
typedef CHUNK_OBJ* tableau_slice_p;


struct tableau_t {
    size_t n_qubits;
    size_t slice_len;
    void* chunks; // Pointer to allocated chunks
    tableau_slice_p* slices_x; // Slice representation pointers 
    tableau_slice_p* slices_z; // Slice representation pointers 
    tableau_slice_p phases; // Phase terms
    bool orientation; // Row or column major order
};
typedef struct tableau_t tableau_t;

/*
 * tableau_create 
 * Constructor class for tableau  
 * :: n_qubits :: Number of qubits   
 * Acts in place and frees all attributes associated with the tableau object
 */
tableau_t* tableau_create(const size_t n_qubits);


/*
 * tableau_destroy 
 * Destructor class for tableau  
 * :: tableau_t* tab :: Tableau to be freed 
 * Acts in place and frees all attributes associated with the tableau object
 */
void tableau_destroy(tableau_t* tab);


/*
 * slice_set_bit
 * Sets a bit in a slice 
 * :: slice : struct tableau_slice* :: The slice object that the bit should be set for
 * :: index : const size_t :: The index to be set
 * :: value : const uint8_t :: The value to set
 * This function acts in place on the slice object
 *
 * This method has both an explicit inline implementation in the local module, and an exposed 
 * non static inlined method. 
 * The enforced behaviour of static inline is only guaranteed for gcc
 */
void slice_set_bit(tableau_slice_p slice, const size_t index, const uint8_t value);

/*
 * slice_get_bit
 * Gets a bit from a slice 
 * This method has both an explicit inline implementation in the local module, and an exposed 
 * non static inlined method. 
 * The enforced behaviour of static inline is only guaranteed for gcc 
 */
uint8_t slice_get_bit(tableau_slice_p slice, const size_t index);

/*
 * tableau_print 
 * Inefficient method for printing a tableau 
 * :: tab : const tableau_t* :: Tableau to print
 */
void tableau_print(const tableau_t* tab);

/*
 * tableau_transpose
 * Transposes a tableau
 * :: tab : tableau_t* :: Tableau to transpose
 * Flips the orientation field and the member fields
 * This flips the alignment of the cache lines
 * TODO simd port swap based nlogn bitvector transpose 
 */
void tableau_transpose(tableau_t* tab);

/*
 * tableau_rowsum
 * Performs a rowsum between two rows of stabilisers 
 * :: tab : tableau_t const* :: Tableau object
 * :: ctrl : const size_t :: Control of the rowsum
 * :: targ : const size_t :: Target of the rowsum
 */
void tableau_rowsum(tableau_t const* tab, const size_t ctrl, const size_t targ);

/*
 * tableau_hadamard
 * Performs a Hadamard between two columns of stabilisers 
 * :: tab : tableau_t const* :: Tableau object
 * :: targ : const size_t :: Target of the Hadamard 
 * Provides both a module specific __inline method along with an exposed tableau_hadamard function 
 */
void tableau_hadamard(tableau_t const* tab, const size_t targ);

/*
 * tableau_cnot
 * Performs a CNOT between two columns of stabilisers 
 * :: tab : tableau_t const* :: Tableau object
 * :: ctrl : const size_t :: Index of control qubit
 * :: targ : const size_t :: Index of target qubit
 */
void tableau_cnot(tableau_t const* tab, const size_t ctrl, const size_t targ);

/*
 * tableau_slice_empty_x
 * Fast operation for checking if an x slice is empty
 *  :: tab : const tableau_t* :: The tableau object
 *  :: idx : const size_t :: Index of the slice
 */
bool tableau_slice_empty_x(const tableau_t* tab, size_t idx);

/*
 * tableau_slice_empty_z
 * Fast operation for checking if an x slice is empty
 *  :: tab : const tableau_t* :: The tableau object
 *  :: idx : const size_t :: Index of the slice
 */
bool tableau_slice_empty_z(const tableau_t* tab, size_t idx);

/*
 * tableau_ctz
 * Fast operation for checking if an x slice is empty
 *  :: tab : const tableau_t* :: The tableau object
 *  :: idx : const size_t :: Index of the slice
 */
size_t tableau_ctz(CHUNK_OBJ* slice, const size_t len);


#endif 
