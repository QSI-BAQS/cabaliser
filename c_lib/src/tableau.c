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
    DPRINT(DEBUG_1, "Allocating %zu qubit tableau\n", n_qubits);

    // The extra chunk is a 64 byte region that we can use for cache line alignment 
    const size_t slice_len_bytes = SLICE_LEN_BYTES(n_qubits, CACHE_SIZE); 
    const size_t tableau_bytes = slice_len_bytes * n_qubits * 2; 

    // Construct memaligned bitmap
    void* tableau_bitmap = NULL;

    int err_code = posix_memalign(&tableau_bitmap, CACHE_SIZE, tableau_bytes); 
    assert(0 == err_code);

    // Set map to all zeros
    memset(tableau_bitmap, 0x00, tableau_bytes);
    DPRINT(DEBUG_2, "\tAllocated %ld bytes for tableau\n", tableau_bytes);
    DPRINT(DEBUG_2, "\tSlices contain %zu bytes\n", slice_len_bytes);

    // Construct start of X and Z segments 
    void* z_start = tableau_bitmap;
    void* x_start = (uint8_t*)tableau_bitmap + slice_len_bytes * n_qubits; 
    
    // Slice tracking pointers 
    void* slice_ptrs_z = malloc(sizeof(void*) * n_qubits); 
    void* slice_ptrs_x = malloc(sizeof(void*) * n_qubits); 

    void* phases = NULL;
    err_code = posix_memalign(&phases, CACHE_SIZE, slice_len_bytes); 
    memset(phases, 0x00, slice_len_bytes);

    // Create the tableau struct and assign variables   
    tableau_t* tab = malloc(sizeof(tableau_t)); 
    tab->n_qubits = n_qubits;
    tab->slice_len = slice_len_bytes;
    tab->chunks = tableau_bitmap; 
    tab->slices_x = slice_ptrs_x;
    tab->slices_z = slice_ptrs_z;
    tab->orientation = COL_MAJOR;
    tab->phases = phases;

    for (size_t i = 0; i < n_qubits; i++)
    {   
        uint8_t* ptr_z = z_start + (i * slice_len_bytes);
        uint8_t* ptr_x = x_start + (i * slice_len_bytes);

        tab->slices_z[i] = (tableau_slice_p)ptr_z; 
        // One write per cache line entry, should be collision free 
        slice_set_bit(tab->slices_z[i], i, 1); 
        tab->slices_x[i] = (tableau_slice_p)ptr_x; 
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
    tab->slice_len = SLICE_LEN_BYTES(n_qubits, CACHE_SIZE); 
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

    // TODO SIMD this by chunks  
    for (size_t i = 0; i < tab->n_qubits; i++)
    {
        bit_z = __inline_slice_get_bit(tab->slices_z[i], targ); 
        bit_x = __inline_slice_get_bit(tab->slices_x[i], targ); 
        uint8_t bit_phase = ((uint8_t)bit_z & (uint8_t)bit_x) ^ __inline_slice_get_bit(tab->phases, i);
        __inline_slice_set_bit(tab->phases, i, bit_phase);
        __inline_slice_set_bit(tab->slices_z[i], targ, bit_x); 
        __inline_slice_set_bit(tab->slices_x[i], targ, bit_z); 
    }
    return;
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
 * Transposes the tableau
 */
void tableau_transpose_slices(tableau_t* tab, uint64_t** slices); 
void tableau_transpose(tableau_t* tab)
{

    if (tab->n_qubits < 64)
    {
        tableau_transpose_naive(tab);
        return;
    }

    tableau_transpose_slices(tab, tab->slices_x);
    tableau_transpose_slices(tab, tab->slices_z);
}
void tableau_transpose_slices(tableau_t* tab, uint64_t** slices) 
{
    const size_t chunk_elements =  tab->n_qubits / (8 * sizeof(uint64_t)); 
    const size_t remainder_elements = tab->n_qubits % 64; 


    uint64_t* src_ptr[64] = {NULL};
    uint64_t* targ_ptr[64] = {NULL};
 
    for (size_t col = 0; col < chunk_elements; col++) 
    {
        for (size_t row = col + 1; row < chunk_elements; row++) 
        {
             // TODO Vectorise this
            for (size_t i = 0; i < 64; i++)
            {
                src_ptr[i] = slices[i + (64 * col)] + row;
                targ_ptr[i] = slices[i + (64 * row)] + col;
            }
            simd_transpose_64x64(src_ptr, targ_ptr);
        }
    }

    // Inplace diagonal
    for (size_t col = 0; col < chunk_elements; col++)
    {
        for (size_t i = 0; i < 64; i++)
        {
            src_ptr[i] = slices[i + (64 * col)] + col;
        }
        simd_transpose_64x64_inplace(src_ptr);
    }

    // Doing the remainder naively
    for (size_t i = chunk_elements * 64; i < tab->n_qubits; i++)
    {
        // Inner loop should run along the current orientation, and hence along the cache lines 
        tableau_slice_p ptr_x = slices[i]; 
   
        for (size_t j = 0; j < tab->n_qubits - (remainder_elements - (i - chunk_elements * 64)); j++)
        {
            uint8_t val_a = __inline_slice_get_bit(ptr_x, j); 
            uint8_t val_b = __inline_slice_get_bit(slices[j], i); 

            __inline_slice_set_bit(ptr_x, j, val_b);
            __inline_slice_set_bit(slices[j], i, val_a);
        }    
    }
   
    return;
}

/*
 * Naive transpose method retained for comparisons when testing
 */
void tableau_transpose_naive(tableau_t* tab)
{

    for (size_t i = 0; i < tab->n_qubits; i++)
    {
        // Inner loop should run along the current orientation, and hence along the cache lines 
        tableau_slice_p ptr_x = tab->slices_x[i]; 
        tableau_slice_p ptr_z = tab->slices_z[i];
    
        for (size_t j = i + 1; j < tab->n_qubits; j++)
        {
            uint8_t val_a = __inline_slice_get_bit(ptr_x, j); 
            uint8_t val_b = __inline_slice_get_bit(tab->slices_x[j], i); 

            __inline_slice_set_bit(ptr_x, j, val_b);
            __inline_slice_set_bit(tab->slices_x[j], i, val_a);

            val_a = __inline_slice_get_bit(ptr_z, j); 
            val_b = __inline_slice_get_bit(tab->slices_z[j], i); 
         
            __inline_slice_set_bit(ptr_z, j, val_b); 
            __inline_slice_set_bit(tab->slices_z[j], i, val_a); 
        }    
    }
    return;
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
 * tableau_print_phases 
 * Inefficient method for printing a tableau 
 * :: tab : const tableau_t* :: Tableau to print
 */
void tableau_print_phases(const tableau_t* tab)
{
    for (size_t i = 0; i < tab->n_qubits; i++)
    {
        printf("|");
        printf("%d", slice_get_bit(tab->phases, i));
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
bool __inline_slice_empty(tableau_slice* slice, const size_t slice_len)
{
    size_t idx = tableau_ctz(slice, slice_len);  
    return (CTZ_SENTINEL == idx); 
}
bool slice_empty(tableau_slice* slice, const size_t slice_len)
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
    #pragma GCC unroll 8
    for (size_t i = 0;
         i < slice_len;
         i += CHUNK_STRIDE)
    {
        DPRINT(DEBUG_3, 
            "%p %lu\n",
            slice + i,
            *(slice + i));
           
        if (0 < CHUNK_IDX(slice, i))
        {
            return CHUNK_SIZE_BITS * (i / CHUNK_STRIDE) + __CHUNK_CTZ(CHUNK_IDX(slice, i));
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
    uint64_t* slice_x_i = tab->slices_x[i];
    uint64_t* slice_x_j = tab->slices_x[j];
    uint64_t* slice_z_i = tab->slices_z[i];
    uint64_t* slice_z_j = tab->slices_z[j];

    printf("SWAPPING: %zu %zu\n", i, j);

    const size_t term = SLICE_LEN_BYTES(tab->n_qubits, sizeof(uint64_t));
    #pragma GCC unroll 8
    for (size_t idx = 0; idx < term; idx++)
    {
        slice_x_i[idx] ^= slice_x_j[idx];
        slice_x_j[idx] ^= slice_x_i[idx];
        slice_x_i[idx] ^= slice_x_j[idx];

        slice_z_i[idx] ^= slice_z_j[idx];
        slice_z_j[idx] ^= slice_z_i[idx];
        slice_z_i[idx] ^= slice_z_j[idx];
    }

    uint8_t phase_i = slice_get_bit(tab->phases, i); 
    uint8_t phase_j = slice_get_bit(tab->phases, j); 
    slice_set_bit(tab->phases, i, phase_j); 
    slice_set_bit(tab->phases, j, phase_i); 

    return;
}

/*
 * tableau_idx_swap_ptr
 * Swaps indicies over both the X and Z slices  
 * :: tab : tableau_t* :: Tableau object to swap over 
 * :: i :: const size_t :: Index to swap 
 * :: j :: const size_t :: Index to swap
 * Acts in place on the tableau 
 */
void tableau_idx_swap_ptr(tableau_t* tab, const size_t i, const size_t j)
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
    uint64_t* slice_x_i = tab->slices_x[i];
    uint64_t* slice_x_j = tab->slices_x[j];
    uint64_t* slice_z_i = tab->slices_z[i];
    uint64_t* slice_z_j = tab->slices_z[j];

    printf("SWAPPING: %zu %zu\n", i, j);

    const size_t term = SLICE_LEN_BYTES(tab->n_qubits, sizeof(uint64_t));
    #pragma GCC unroll 8
    for (size_t idx = 0; idx < term; idx++)
    {
        slice_x_i[idx] ^= slice_x_j[idx];
        slice_x_j[idx] ^= slice_x_i[idx];
        slice_x_i[idx] ^= slice_x_j[idx];

        slice_z_i[idx] ^= slice_z_j[idx];
        slice_z_j[idx] ^= slice_z_i[idx];
        slice_z_i[idx] ^= slice_z_j[idx];
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
void tableau_rowsum(tableau_t* tab, const size_t ctrl, const size_t targ)
{
    void* slice_ctrl_x = tab->slices_x[ctrl];
    void* slice_targ_x = tab->slices_x[targ];
    void* slice_ctrl_z = tab->slices_z[ctrl];
    void* slice_targ_z = tab->slices_z[targ];

    int8_t phase = simd_rowsum(
        tab->slice_len,
        slice_ctrl_x,
        slice_ctrl_z,
        slice_targ_x,
        slice_targ_z);

    int8_t c_phase = slice_get_bit(tab->phases, ctrl) << 1;
    int8_t t_phase = slice_get_bit(tab->phases, targ) << 1;

    // https://arxiv.org/pdf/quant-ph/0406196 Page 4
    phase = ((c_phase + t_phase + phase) % 4) >> 1;

    slice_set_bit(tab->phases, targ, phase);
}


/*
 * Calculates the rowsum starting from an offset 
 * :: tab : tableau_t* :: Tableau object
 * :: ctrl : const size_t :: Control qubit
 * :: targ : const size_t :: Target qubit
 * :: offset : const size_t :: Offset qubit index 
 */
static inline
void __inline_tableau_rowsum_offset(tableau_t* tab, const size_t ctrl, const size_t targ, const size_t offset)
{
    const size_t offset_bytes = 8 * (offset / 64); 

    tableau_slice* slice_ctrl_x = (tableau_slice*)((void*)(tab->slices_x[ctrl]) + offset_bytes); 
    tableau_slice* slice_ctrl_z = (tableau_slice*)((void*)(tab->slices_z[ctrl]) + offset_bytes); 
    tableau_slice* slice_targ_x = (tableau_slice*)((void*)(tab->slices_x[targ]) + offset_bytes); 
    tableau_slice* slice_targ_z = (tableau_slice*)((void*)(tab->slices_z[targ]) + offset_bytes); 

    int8_t phase = simd_rowsum_cnf(
        tab->slice_len - offset_bytes,
        slice_ctrl_x,
        slice_ctrl_z,
        slice_targ_x,
        slice_targ_z);

    // Inlining will remove this as a blocking load
    int8_t c_phase = slice_get_bit(tab->phases, ctrl) << 1;
    int8_t t_phase = slice_get_bit(tab->phases, targ) << 1;

    // https://arxiv.org/pdf/quant-ph/0406196 Page 4
    phase = ((c_phase + t_phase + phase) % 4) >> 1;

    slice_set_bit(tab->phases, targ, phase);
}
void tableau_rowsum_offset(tableau_t* tab, const size_t ctrl, const size_t targ, const size_t offset)
{
    __inline_tableau_rowsum_offset(tab, ctrl, targ, offset);
}
