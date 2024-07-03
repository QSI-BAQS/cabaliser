#include "aligned_chunk.h"


/*
 * tableau_create 
 * Constructor class for tableau  
 * :: n_qubits :: Number of qubits   
 * Acts in place and frees all attributes associated with the tableau object
 */
tableau_t* tableau_create(const size_t n_qubits)
{
    DPRINT(DEBUG_1, "Allocating %ld qubit tableau\n", n_qubits);

    // The extra chunk is a 64 byte region that we can use for cache line alignment 
    const size_t slice_len = SLICE_LEN(n_qubits) * CACHE_SIZE;

    // Construct memaligned bitmap
    void* tableau_bitmap = NULL;
    const size_t tableau_bytes = slice_len * n_qubits * 2;
    posix_memalign(&tableau_bitmap, CACHE_SIZE, tableau_bytes); 

    // Set map to all zeros
    memset(tableau_bitmap, 0x00, tableau_bytes);
    DPRINT(DEBUG_2, "\tAllocated %ld bytes for tableau\n", slice_len * n_qubits * 2);


    // Construct start of X and Z segments 
    void* z_start = tableau_bitmap;
    void* x_start = tableau_bitmap + slice_len * n_qubits; 
    
    // Slice tracking pointers 
    void* slice_ptrs_z = malloc(sizeof(void*) * n_qubits); 
    void* slice_ptrs_x = malloc(sizeof(void*) * n_qubits); 

    // Create the tableau struct and assign variables   
    tableau_t* tab = malloc(sizeof(tableau_t)); 
    tab->n_qubits = n_qubits;
    tab->chunks = tableau_bitmap; 
    tab->slices_x = slice_ptrs_x;
    tab->slices_z = slice_ptrs_z;
    tab->orientation = COL_MAJOR;

    // Set up slice pointers 
    for (size_t i = 0; i < n_qubits; i++)
    {   
        tab->slices_x[i] = x_start + slice_len * i; 
        tab->slices_z[i] = z_start + slice_len * i; 
        slice_set_bit(tab->slices_z[i], i , 1); 
    }
    return tab;
}


/*
 * tableau_destroy 
 * Destructor class for tableau  
 * :: tableau_t* tab :: Tableau to be freed 
 * Acts in place and frees all attributes associated with the tableau object
 */
void tableau_destroy(tableau_t* tab)
{
    DPRINT(DEBUG_1, "Freeing %ld qubit tableau\n", tab->n_qubits);

    free(tab->slices_x);
    free(tab->slices_z);
    free(tab->chunks);
    free(tab);
    return;
}


void slice_set_bit(struct tableau_slice* slice, const size_t index, uint8_t value)
{
    CHUNK_OBJ mask = (1ull & value) << (index % sizeof(CHUNK_OBJ));
    ((CHUNK_OBJ*)slice)[index / sizeof(CHUNK_OBJ)] &= mask; 
    return; 
}
