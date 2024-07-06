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
static inline
void __inline_slice_set_bit(
    struct tableau_slice const* slice,
    const size_t index,
    const uint8_t value)
{
    CHUNK_OBJ mask = ~((~((CHUNK_OBJ)0)) ^ ((CHUNK_OBJ)(value & 1) << (index % sizeof(CHUNK_OBJ))));  
    ((CHUNK_OBJ*)slice)[index / sizeof(CHUNK_OBJ)] |= mask; 
    DPRINT(DEBUG_3, "\t\tSetting bit using mask %lu\n", mask);
    DPRINT(DEBUG_3, "\t\tSlice %lu\n", ((CHUNK_OBJ*)slice)[index / sizeof(CHUNK_OBJ)]);
    
    return; 
}
void slice_set_bit(
    struct tableau_slice const* slice,
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
static inline
uint8_t __inline_slice_get_bit(
    const struct tableau_slice* slice,
    const size_t index)
{
    CHUNK_OBJ mask = 1ull << (index % sizeof(CHUNK_OBJ));
    return !!(((CHUNK_OBJ*)slice)[index / sizeof(CHUNK_OBJ)] & mask); 
} 
// Non-inlined wrapper
uint8_t slice_get_bit(
    const struct tableau_slice* slice,
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

    void* phases = malloc(sizeof(slice_len));

    // Create the tableau struct and assign variables   
    tableau_t* tab = malloc(sizeof(tableau_t)); 
    tab->n_qubits = n_qubits;
    tab->slice_len = SLICE_LEN(n_qubits);
    tab->chunks = tableau_bitmap; 
    tab->slices_x = slice_ptrs_x;
    tab->slices_z = slice_ptrs_z;
    tab->orientation = COL_MAJOR;
    tab->phases = phases;

    // Set up slice pointers 
    #pragma GCC unroll 8 
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
    free(tab->phases);
    free(tab);
    return;
}


/*
 * tableau_cnot
 * Performs a CNOT between two columns of stabilisers 
 * :: tab : tableau_t const* :: Tableau object
 * :: ctrl : const size_t :: Index of control qubit
 * :: targ : const size_t :: Index of target qubit
 */
void tableau_cnot(tableau_t const* tab, const size_t ctrl, const size_t targ)
{
    CHUNK_OBJ* ctrl_slice_x = (CHUNK_OBJ*)(tab->slices_x[ctrl]); 
    CHUNK_OBJ* targ_slice_x = (CHUNK_OBJ*)(tab->slices_x[targ]); 
    CHUNK_OBJ* ctrl_slice_z = (CHUNK_OBJ*)(tab->slices_z[targ]); 
    CHUNK_OBJ* targ_slice_z = (CHUNK_OBJ*)(tab->slices_z[ctrl]); 

    DPRINT(DEBUG_3, "\t\tCNOT on slices %lu\n", tab->slice_len); 
    #pragma GCC unroll 8
    for (size_t i = 0; i < tab->slice_len; i++)
    {
    DPRINT(DEBUG_3, "\t\t%lu %lu\n", targ_slice_x[i], ctrl_slice_x[i]); 

        targ_slice_x[i] ^= ctrl_slice_x[i];
        targ_slice_z[i] ^= ctrl_slice_z[i];
    DPRINT(DEBUG_3, "\t\t%lu %lu\n", targ_slice_x[i], ctrl_slice_x[i]); 

    }   
}


/*
 * tableau_hadamard
 * Performs a Hadamard between two columns of stabilisers 
 * :: tab : tableau_t const* :: Tableau object
 * :: targ : const size_t :: Target of the Hadamard 
 * Provides both a module specific __inline method along with an exposed tableau_hadamard function 
 */
static inline
void __inline_tableau_hadamard(tableau_t const* tab, const size_t targ)
{
    void* ptr = tab->slices_x[targ];
    tab->slices_x[targ] = tab->slices_z[targ];
    tab->slices_z[targ] = ptr;
}
void tableau_hadamard(tableau_t const* tab, const size_t targ)
{
    __inline_tableau_hadamard(tab, targ);
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
        struct tableau_slice* ptr_x = tab->slices_x[i]; 
        struct tableau_slice* ptr_z = tab->slices_z[i];
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
        struct tableau_slice* ptr_x = tab->slices_x[i]; 
        struct tableau_slice* ptr_z = tab->slices_z[i];
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
        struct tableau_slice* ptr_x = tab->slices_x[i]; 
        struct tableau_slice* ptr_z = tab->slices_z[i];
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
 * tableau_parse_instructions
 * Ingests an array of instructions and implements them  
 * Recommended for use with a threadpool and limiting instructions to acting on a single qubit   
 */
void tableau_parse_instructions(
    tableau_t const* tab,
    const instruction_t* inst,
    const size_t n_instructions) 
{
    //#pragma GCC unroll 8
    for (size_t i = 0; i < n_instructions; i++)
    {
        return;
    } 
    return;
}

/*
 * slice_empty
 * Fast operation for checking if a slice is empty
 * Exposes functions tableau_slice_empty_x and tableau_slice_empty_z which wrap this function
 * :: slice : const struct tableau_slice* :: The slice
 * :: len : const size_t :: Length of the slice in elements of CHUNK_OBJ
 */
static inline
bool __inline_slice_empty(const struct tableau_slice* slice, const size_t len)
{
    bool empty_slice = 0; 
    for (const struct tableau_slice* ptr = slice;
         ptr < slice + len;
         ptr++)
    {

        for (size_t offset = 0;
             offset < CACHE_CHUNKS;
             offset++)
        {
            empty_slice |= ((((CHUNK_OBJ*)ptr) + offset) > 0);
        }
       
        // Checking if constantly leads to slowdowns, collect and then check  
        if (empty_slice)
        {
            return 0;
        }
    }
    return 1;
}
bool slice_empty(const struct tableau_slice* slice, const size_t len)
{
    return __inline_slice_empty(slice, len);
}

/*
 * tableau_slice_empty_x
 * Fast operation for checking if an x slice is empty
 *  :: tab : const tableau_t* :: The tableau object
 *  :: idx : const size_t :: Index of the slice
 */
bool tableau_slice_empty_x(const tableau_t* tab, size_t idx)
{
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

