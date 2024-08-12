#include "tableau.h"

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
void slice_set_bit(
    tableau_slice_p slice,
    const size_t index,
    const uint8_t value)
{
    // static inline forces this expansion
    __inline_slice_set_bit(slice, index, value);
}

/*
 * slice_get_bit
 * Gets a bit from a slice 
 * This method has both an explicit inline implementation in the local module, and an exposed 
 * non static inlined method. 
 * The enforced behaviour of static inline is only guaranteed for gcc 
 */
uint8_t slice_get_bit(
    tableau_slice_p slice,
    const size_t index)
{
    return __inline_slice_get_bit(slice, index);
}


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

    size_t slice_len_bytes = n_qubits / 8 + !!(n_qubits % 8);
    const size_t slice_len_sized = slice_len_bytes / sizeof(CHUNK_OBJ) + !!(slice_len_bytes % sizeof(CHUNK_OBJ)); 
    const size_t slice_len_cache = slice_len_bytes / CACHE_SIZE + !!(slice_len_bytes % CACHE_SIZE); 

    slice_len_bytes = slice_len_cache * CACHE_SIZE; 

    assert(slice_len_sized * sizeof(size_t) <= slice_len_bytes);


    // Construct memaligned bitmap
    void* tableau_bitmap = NULL;
    const size_t tableau_bytes = slice_len_bytes * n_qubits * 2;
    int err_code = posix_memalign(&tableau_bitmap, CACHE_SIZE, tableau_bytes); 
    assert(0 == err_code);

    // Set map to all zeros
    memset(tableau_bitmap, 0x00, tableau_bytes);
    DPRINT(DEBUG_2, "\tAllocated %ld bytes for tableau\n", tableau_bytes);
    DPRINT(DEBUG_2, "\t\t Slices: %lu bytes, %lu cache chunks, %lu size_t chunks\n", slice_len_bytes, slice_len_cache, slice_len_sized);


    // Construct start of X and Z segments 
    void* z_start = tableau_bitmap;
    void* x_start = tableau_bitmap + slice_len_bytes * n_qubits; 
    
    // Slice tracking pointers 
    void* slice_ptrs_z = malloc(sizeof(void*) * n_qubits); 
    void* slice_ptrs_x = malloc(sizeof(void*) * n_qubits); 

    void* phases = NULL;
    err_code = posix_memalign(&phases, CACHE_SIZE, slice_len_bytes); 
    memset(phases, 0x00, slice_len_bytes);

    // Create the tableau struct and assign variables   
    tableau_t* tab = malloc(sizeof(tableau_t)); 
    tab->n_qubits = n_qubits;
    tab->slice_len = slice_len_sized;
    tab->chunks = tableau_bitmap; 
    tab->slices_x = slice_ptrs_x;
    tab->slices_z = slice_ptrs_z;
    tab->orientation = COL_MAJOR;
    tab->phases = phases;

    // Set up slice pointers 
    uint8_t* ptr_z = (uint8_t*)z_start;
    uint8_t* ptr_x = (uint8_t*)x_start;

    #pragma GCC unroll 8 
    for (size_t i = 0; i < n_qubits; i++)
    {   
        tab->slices_z[i] = (tableau_slice_p)ptr_z; 
        slice_set_bit(tab->slices_z[i], i, 1); 
        tab->slices_x[i] = (tableau_slice_p)ptr_x; 

        ptr_z += slice_len_bytes;
        ptr_x += slice_len_bytes;
    }
    return tab;
}

/*
 * tableau_set_n_qubits
 * Truncates the tableau to a set number of qubits
 * :: tab : tableau_t* :: The tableau
 * :: n_qubits : const size_t :: The number of qubits
 * Acts in place on the tableau
 * There is an implicit assumption that the number of qubits should not be greater than the initially allocated number of qubits
 */
void tableau_set_n_qubits(tableau_t* tab, const size_t n_qubits)
{
    tab->n_qubits = n_qubits; 
    tab->slice_len = SLICE_LEN_SIZE_T(n_qubits); 
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
    free(tab->phases);
    free(tab);
    return;
}


/*
 * tableau_transverse_hadamard
 * Applies a hadamard when transposed 
 * :: tab : tableau_t*  :: Tableau object
 * :: c_que :  clifford_queue_t* :: Clifford queue 
 * :: i : const size_t :: Index to target 
 *
 */
void tableau_transverse_hadamard(tableau_t const* tab, const size_t targ)
{ 
    uint8_t bit_x = 0;
    uint8_t bit_z = 0;
    // TODO Vectorise this
    #pragma GCC ivdep  
    for (size_t i = 0; i < tab->n_qubits; i++)
    {
        bit_z = __inline_slice_get_bit(tab->slices_z[i], targ); 
        bit_x = __inline_slice_get_bit(tab->slices_x[i], targ); 

        __inline_slice_set_bit(tab->slices_z[i], targ, bit_x); 
        __inline_slice_set_bit(tab->slices_x[i], targ, bit_z); 
    }

    return;
}

/*
 * tableau_rowsum
 * Performs a rowsum between two rows of stabilisers 
 * :: tab : tableau_t const* :: Tableau object
 * :: ctrl : const size_t :: Control of the rowsum
 * :: targ : const size_t :: Target of the rowsum
 */
void tableau_rowsum(tableau_t const* tab, const size_t ctrl, const size_t targ)
{
    DEBUG_CHECK(ROW_MAJOR == tab->orientation);
    CHUNK_OBJ* ctrl_slice_x = (CHUNK_OBJ*)(tab->slices_x[ctrl]); 
    CHUNK_OBJ* targ_slice_x = (CHUNK_OBJ*)(tab->slices_z[targ]); 
    CHUNK_OBJ* ctrl_slice_z = (CHUNK_OBJ*)(tab->slices_z[ctrl]); 
    CHUNK_OBJ* targ_slice_z = (CHUNK_OBJ*)(tab->slices_z[targ]); 

    #ifdef UNROLL
    #pragma GCC unroll 8 
    #endif
    #pragma GCC ivdep
    for (size_t i = 0; i < tab->slice_len; i++)
    {
        targ_slice_x[i] ^= ctrl_slice_x[i];
        targ_slice_z[i] ^= ctrl_slice_z[i];
    }   
}

void tableau_col_elim_X(tableau_t const* tab, const size_t idx)
{
    #define VECTOR_COLLECT 8 
    size_t collected = 0;   
    size_t* rows = {0}; 
    DEBUG_CHECK(ROW_MAJOR == tab->orientation);

    for (size_t j = 0; j < idx;)
    {
        // Collect Rows
        while ((j < idx) && (VECTOR_COLLECT > collected))
        {
            if (__inline_slice_get_bit(tab->slices_x[j], idx))
            {
                rows[collected] = j;  
                collected++;
            }
            j++;
        }
        // Rowsum 
        // TODO Multithread this 
        if (collected > 0)
        {
            // Assume elements left of the targeted chunk are already zeroed by previous elim operations 
            // Apply to X and Z chunks separately to double cache lifetime 
            // Z elements
            for (size_t jdx = idx / CHUNK_SIZE_BITS; jdx < SLICE_LEN_CACHE(tab->n_qubits); jdx++)
            {
                // TODO Vectorise  and distribute this loop
                for  (size_t col_idx = 0; col_idx < collected; col_idx += 1)
                {
                    tab->slices_z[rows[col_idx]][jdx] ^= tab->slices_z[idx][jdx];   
                } 
            }        
             
            // X elements 
            for (size_t jdx = idx / CHUNK_SIZE_BITS; jdx < SLICE_LEN_CACHE(tab->n_qubits); jdx++)
            {
                for  (size_t col_idx = 0; col_idx < collected; col_idx += 1)
                {
                    tab->slices_x[rows[col_idx]][jdx] ^= tab->slices_z[idx][jdx];   
                } 
            }         
        }  
    }   
}

/*
 * tableau_transpose
 * Transposes a tableau
 * :: tab : tableau_t* :: Tableau to transpose
 * Flips the orientation field and the member fields
 * This flips the alignment of the cache lines
 * TODO simd port swap based nlogn bitvector transpose 
 */
void tableau_transpose(tableau_t* tab)
{
    // Flip orientation
    tab->orientation ^= 1;

    // Outer loop should jump between cache lines 
    for (size_t i = 0; i < tab->n_qubits; i++)
    {
        // Inner loop should run along the current orientation, and hence along the cache lines 
        tableau_slice_p ptr_x = tab->slices_x[i]; 
        tableau_slice_p ptr_z = tab->slices_z[i];
        #pragma GCC unroll 8
        for (size_t j = i + 1; j < tab->n_qubits; j++)
        {
            uint8_t val_a = __inline_slice_get_bit(ptr_x, j); 
            uint8_t val_b = __inline_slice_get_bit(tab->slices_x[j], i); 
         
            __inline_slice_set_bit(ptr_x, j, val_b); 
            __inline_slice_set_bit(tab->slices_x[j], j, val_a); 

            val_a = __inline_slice_get_bit(ptr_z, j); 
            val_b = __inline_slice_get_bit(tab->slices_z[j], i); 
         
            __inline_slice_set_bit(ptr_z, j, val_b); 
            __inline_slice_set_bit(tab->slices_z[j], j, val_a); 
        }    
    }
}
/*
 * gs_tableau_transpose
 * Transposes a tableau for a known graph state
 * The only difference is this guarantees the mututal independence of the input qubits 
 * These may be excluded from the transpose operation
 */
void gs_tableau_transpose(tableau_t* tab, const size_t input_qubits)
{
    // Flip orientation
    tab->orientation ^= 1;

    // Prevent bad inputs
    DEBUG_CHECK(input_qubits <= tab->n_qubits);

    // Independent input qubits  
    // Outer loop should jump between cache lines 
    for (size_t i = 0; i < input_qubits; i++)
    {
        // Inner loop should run along the current orientation, and hence along the cache lines 
        tableau_slice_p ptr_x = tab->slices_x[i]; 
        tableau_slice_p ptr_z = tab->slices_z[i];
        #pragma GCC unroll 8
        for (size_t j = input_qubits; j < tab->n_qubits; j++)
        {
            uint8_t val_a = __inline_slice_get_bit(ptr_x, j); 
            uint8_t val_b = __inline_slice_get_bit(tab->slices_x[j], i); 
         
            __inline_slice_set_bit(ptr_x, j, val_b); 
            __inline_slice_set_bit(tab->slices_x[j], j, val_a); 

            val_a = __inline_slice_get_bit(ptr_z, j); 
            val_b = __inline_slice_get_bit(tab->slices_z[j], i); 
         
            __inline_slice_set_bit(ptr_z, j, val_b); 
            __inline_slice_set_bit(tab->slices_z[j], j, val_a); 
        }    
    }
    
    // Outer loop should jump between cache lines 
    for (size_t i = input_qubits; i < tab->n_qubits; i++)
    {
        // Inner loop should run along the current orientation, and hence along the cache lines 
        tableau_slice_p ptr_x = tab->slices_x[i]; 
        tableau_slice_p ptr_z = tab->slices_z[i];
        #pragma GCC unroll 8
        for (size_t j = i + 1; j < tab->n_qubits; j++)
        {
            uint8_t val_a = __inline_slice_get_bit(ptr_x, j); 
            uint8_t val_b = __inline_slice_get_bit(tab->slices_x[j], i); 
         
            __inline_slice_set_bit(ptr_x, j, val_b); 
            __inline_slice_set_bit(tab->slices_x[j], j, val_a); 

            val_a = __inline_slice_get_bit(ptr_z, j); 
            val_b = __inline_slice_get_bit(tab->slices_z[j], i); 
         
            __inline_slice_set_bit(ptr_z, j, val_b); 
            __inline_slice_set_bit(tab->slices_z[j], j, val_a); 
        }    
    }

}


/*
 * tableau_print 
 * Inefficient method for printing a tableau 
 * :: tab : const tableau_t* :: Tableau to print
 */
void tableau_print(const tableau_t* tab)
{
    // Col major order so this is very inefficient
    
    for (size_t i = 0; i < tab->n_qubits; i++)
    {
       printf("|");

        for (size_t j = 0; j < tab->n_qubits; j++)
{
    printf("%d", slice_get_bit(tab->slices_x[j], i));
}
     printf("|");

        for (size_t j = 0; j < tab->n_qubits; j++)
{
    printf("%d", slice_get_bit(tab->slices_z[j], i));
}
    printf("|\n");
    }
}


/*
 * slice_empty
 * Fast operation for checking if a slice is empty
 * Exposes functions tableau_slice_empty_x and tableau_slice_empty_z which wrap this function
 * :: slice : const struct tableau_slice* :: The slice
 * :: slice_len : const size_t :: Length of slice in units of size_t
 */
static inline
bool __inline_slice_empty(tableau_slice_p slice, const size_t slice_len)
{
    size_t idx = tableau_ctz(slice, slice_len);  
    return (CTZ_SENTINEL == idx); 
}
bool slice_empty(tableau_slice_p slice, const size_t slice_len)
{
    return __inline_slice_empty(slice, slice_len);
}

/*
 * tableau_slice_empty_x
 * Fast operation for checking if an x slice is empty
 *  :: tab : const tableau_t* :: The tableau object
 *  :: idx : const size_t :: Index of the slice
 */
bool tableau_slice_empty_x(const tableau_t* tab, size_t idx)
{
    DPRINT(DEBUG_3, "\t\tChecking X slice %lu is empty\n", idx);

    return __inline_slice_empty(tab->slices_x[idx], tab->slice_len);
} 


/*
 * tableau_slice_empty_z
 * Fast operation for checking if an x slice is empty
 *  :: tab : const tableau_t* :: The tableau object
 *  :: idx : const size_t :: Index of the slice
 */
bool tableau_slice_empty_z(const tableau_t* tab, size_t idx)
{
    return __inline_slice_empty(tab->slices_z[idx], tab->slice_len);
}


/*
 * tableau_ctz
 * Gets the index of the first non-zero bit in the slice
 * :: tableau_slice_p ::
 */
size_t tableau_ctz(CHUNK_OBJ* slice, const size_t slice_len)
{
    //#pragma GCC unroll 8
    for (size_t i = 0;
         i < slice_len;
         i++)
    {
        DPRINT(DEBUG_3, 
            "%p %lu\n",
            slice + i,
            *(slice + i));
           
        if (0 < *(slice + i))
        {
            return CHUNK_SIZE_BITS * i + __CHUNK_CTZ(*(slice + i));
        }
    }
    return CTZ_SENTINEL;
}


/*
 * tableau_idx_swap 
 * Swaps indicies over both the X and Z slices  
 * :: tab : tableau_t* :: Tableau object to swap over 
 * :: i :: const size_t :: Index to swap 
 * :: j :: const size_t :: Index to swap
 * Acts in place on the tableau 
 */
void tableau_idx_swap(tableau_t* tab, const size_t i, const size_t j)
{
    tableau_slice_p tmp = tab->slices_x[i];
    tab->slices_x[i] = tab->slices_x[j];  
    tab->slices_x[j] = tmp;  

    tmp = tab->slices_z[i];
    tab->slices_z[i] = tab->slices_z[j];  
    tab->slices_z[j] = tmp;  

    return;
}


/*
 * tableau_idx_swap_transverse 
 * Swaps indicies over both the X and Z slices  
 * Also swaps associated phases
 * :: tab : tableau_t* :: Tableau object to swap over 
 * :: i :: const size_t :: Index to swap 
 * :: j :: const size_t :: Index to swap
 * Acts in place on the tableau 
 */
void tableau_idx_swap_transverse(tableau_t* tab, const size_t i, const size_t j)
{
    tableau_slice_p tmp = tab->slices_x[i];
    tab->slices_x[i] = tab->slices_x[j];  
    tab->slices_x[j] = tmp;  

    tmp = tab->slices_z[i];
    tab->slices_z[i] = tab->slices_z[j];  
    tab->slices_z[j] = tmp;  
    
    uint8_t phase_i = slice_get_bit(tab->phases, i); 
    uint8_t phase_j = slice_get_bit(tab->phases, j); 
    slice_set_bit(tab->phases, i, phase_j); 
    slice_set_bit(tab->phases, j, phase_i); 

    return;
}
