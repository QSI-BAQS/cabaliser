#define INSTRUCTIONS_TABLE
#define TABLEAU_OPERATIONS_SRC 

#include "tableau_operations.h"

/*
 * tableau_remove_zero_X_columns
 * Uses hadamards to clear any zeroed X columns
 * :: tab : tableau_t* :: The tableau to act on
 * Acts in place over the tableau
 */
void tableau_remove_zero_X_columns(tableau_t* tab, clifford_queue_t* c_que)
{
    // Can't parallelise as the H operation is already parallel
    for (size_t i = 0; i < tab->n_qubits; i++)
    {
       if (CTZ_SENTINEL == tableau_ctz(tab->slices_x[i], tab->slice_len)) 
       {
         DPRINT(DEBUG_3, "Empty X Col, applying H to %lu\n", i);
         tableau_H(tab, i);
         clifford_queue_local_clifford_right(c_que, _H_, i);
       } 
    } 
    return;
}

/*
 * tableau_Z_block_diagonal
 * Ensures that the Z block of the tableau is diagonal
 * Operation unused as it only applies to the non-transposed matrix
 * :: tab : tableau_t* :: Tableau to act on
 *
 */
void tableau_Z_zero_diagonal(tableau_t* tab, clifford_queue_t* c_que)
{
    for (size_t i = 0; i < tab->n_qubits; i++)
    { 
        if (__inline_slice_get_bit(tab->slices_z[i], i))  
        {
            DPRINT(DEBUG_3, "Zero Diagonal: Phase on %lu ", i);
            tableau_R(tab, i);
            clifford_queue_local_clifford_right(c_que, i, _S_);   
        }
    }
    return;
}

/*
 *
 *
 */
void tableau_X_diag_element(tableau_t* tab, clifford_queue_t* queue, const size_t idx)
{

    for (size_t j = idx + 1; j < tab->n_qubits; j++)
    {
        // Swap stabilisers
        if (1 == __inline_slice_get_bit(tab->slices_x[j], idx))
        {
            DPRINT(DEBUG_3, "Strategy 1: Swapping %lu <-> %lu\n", j, idx);
            tableau_idx_swap_transverse(tab, idx, j);
            return; 
        } 
    } 

    if (1 == __inline_slice_get_bit(tab->slices_z[idx], idx))
    {
        DPRINT(DEBUG_3, "Strategy 2: Applying Hadamard to %lu\n", idx);
        tableau_transverse_hadamard(tab, idx);
        clifford_queue_local_clifford_right(queue, _H_, idx);   
        return;
    }

    for (size_t j = idx + 1; j < tab->n_qubits; j++)
    {
        // Swap stabilisers
        if (1 == __inline_slice_get_bit(tab->slices_z[j], idx))
        {
            // Backup Strategy
            tableau_idx_swap_transverse(tab, idx, j);

            DPRINT(DEBUG_3, "Strategy 3: Applying Hadamard to %lu\n", idx);

            tableau_transverse_hadamard(tab, idx);
            clifford_queue_local_clifford_right(queue, _H_, idx);   
            return; 
        } 
    }
    DPRINT(DEBUG_3, "Applying Hadamard to %lu\n", idx);
    return;
}

void tableau_X_diag_col_upper(tableau_t* tab, const size_t idx)
{
    size_t  j;
    for (j = idx + 1; j < tab->n_qubits; j++) 
    {
        if (1 == __inline_slice_get_bit(tab->slices_x[j], idx))
        {
            DPRINT(DEBUG_3, "Slice XOR Upper: %lu %lu\n", idx, j);
            tableau_slice_xor(tab, idx, j);
        }
    }
    return;
}


void tableau_X_diag_col_lower(tableau_t* tab, const size_t idx)
{
    size_t j;
    CHUNK_OBJ* slice = tab->slices_x[idx];

    for (j = 0; j < idx; j++) 
    {
        if (1 == __inline_slice_get_bit(tab->slices_x[j], idx))
        {
            DPRINT(DEBUG_3, "Slice XOR Lower: %lu %lu\n", idx, j);
            tableau_slice_xor(tab, idx, j);
        }
    }
    return;
}

/*
 * tableau_X_upper_right_triangular
 * Makes the X block upper right triangular 
 * :: tab : tableau_t* :: Tableau to operate on    
 */
void tableau_X_diagonal(tableau_t* tab, clifford_queue_t* c_que)
{
    tableau_transpose(tab);

    for (size_t i = 0; i < tab->n_qubits; i++)
    {
        // X(i, i) != 1 
        if (0 == __inline_slice_get_bit(tab->slices_x[i], i))
        {  
            tableau_X_diag_element(tab, c_que, i);
        }
    }
    return;
}


void tableau_H(tableau_t* tab, const size_t targ)
{
    /*
     * x -> z
     * z -> x
     * r -> r ^ x.z
     */
    void* slice_x = (void*)(tab->slices_x[targ]); 
    void* slice_z = (void*)(tab->slices_z[targ]); 
    void* slice_r = (void*)(tab->phases); 

    for (size_t i = 0; i < tab->n_qubits / 8; i += TABLEAU_SIMD_STRIDE)
    {
        __m256i x = _mm256_load_si256(slice_x + i);   
        __m256i z = _mm256_load_si256(slice_z + i);
        __m256i r = _mm256_load_si256(slice_r + i);

        _mm256_store_si256(slice_x + i, z);
        _mm256_store_si256(slice_z + i, x);
        _mm256_store_si256(slice_r + i, _mm256_xor_si256(r, _mm256_and_si256(x, z)));
    }  
}


void tableau_S(tableau_t* tab, const size_t targ)
{
    /*
     * x -> x  
     * z -> z ^ x
     * r -> r ^ x.z
     */
    void* slice_x = (void*)(tab->slices_x[targ]); 
    void* slice_z = (void*)(tab->slices_z[targ]); 
    void* slice_r = (void*)(tab->phases); 

    for (size_t i = 0; i < tab->n_qubits / 8; i += TABLEAU_SIMD_STRIDE)
    {
        __m256i x = _mm256_load_si256(slice_x + i);
        __m256i z = _mm256_load_si256(slice_z + i);
        __m256i r = _mm256_load_si256(slice_r + i);

        _mm256_store_si256(slice_z + i, _mm256_xor_si256(x, z));
        _mm256_store_si256(slice_r + i, 
            _mm256_xor_si256(r,
                _mm256_and_si256(x, z)
            )
        );
    }
}

void tableau_Z(tableau_t* tab, const size_t targ)
{
/*
 * Doubled S gate
 * S: (r ^= x.z; z ^= x)
 * r1 = r0 ^ (x.z0); z1 = z0 ^ x 
 * r2 = r0 ^ (x.z0) ^ (x.(z0 ^ x)); z2 = z0 ^ x ^ x 
 * r2 = r0 ^ x.(z0 ^ z0 ^ x); z2 = z0 
 * r2 = r0 ^ x; z2 = z0 
 */
    void* slice_x = (void*)(tab->slices_x[targ]); 
    void* slice_z = (void*)(tab->slices_z[targ]); 
    void* slice_r = (void*)(tab->phases); 

    for (size_t i = 0; i < tab->n_qubits / 8; i += TABLEAU_SIMD_STRIDE)
    {
        __m256i x = _mm256_load_si256(slice_x + i);
        __m256i z = _mm256_load_si256(slice_z + i);
        __m256i r = _mm256_load_si256(slice_r + i);

        _mm256_store_si256(
            slice_r + i, 
            _mm256_xor_si256(r, x)
        );
    }
}


void tableau_R(tableau_t* tab, const size_t targ)
{
    /*
     * Triple S gate
     * S : (r ^= x.z; z ^= x)
     * r1 = r0 ^ (x.z0); z1 = z0 ^ x 
     *
     * r2 = r0 ^ (x.z0) ^ (x.(z0 ^ x)); z2 = z0 ^ x ^ x 
     * r2 = r0 ^ x.(z0 ^ z0 ^ x); z2 = z0 
     * r2 = r0 ^ x; z2 = z0 
     *
     * r3 = r0 ^ x ^ x.z2; z3 = z2 ^ x 
     * r3 = r0 ^ x ^ x.z0; z3 = z0 ^ x 
     *
     * R : (r ^= x.~z; z ^= x)
     */

    void* slice_x = (void*)(tab->slices_x[targ]); 
    void* slice_z = (void*)(tab->slices_z[targ]); 
    void* slice_r = (void*)(tab->phases); 

    for (size_t i = 0; i < tab->n_qubits / 8; i += TABLEAU_SIMD_STRIDE)
    {
        __m256i x = _mm256_load_si256(slice_x + i);
        __m256i z = _mm256_load_si256(slice_z + i);
        __m256i r = _mm256_load_si256(slice_r + i);

        _mm256_store_si256(slice_z + i, _mm256_xor_si256(x, z));
        _mm256_store_si256(slice_r + i, 
            _mm256_xor_si256(
            r,
            _mm256_andnot_si256(z, x)
            )
        );
    }
}


void tableau_I(tableau_t* tab, const size_t targ)
{
    return;
}

void tableau_X(tableau_t* tab, const size_t targ)
{
/*
 * HZH Gate
 * Flip X and Z, perform a Z, then flip X and Z again
 * H : (r ^= x.z; x <-> z) 
 * Z : (r ^= x)
 * 
 * H
 * r_1 = r_0 ^ x0.z0; x1 = z0; z1 = x0  
 * 
 * Z
 * r_2 = r_1 ^ x1; x2 = x1; z2 = z1  
 * r_2 = r_0 ^ x0.z0 ^ z0; x2 = z0; z2 = x0  
 *
 * H
 * r_3 = r_0 ^ x0.z0 ^ z0 ^ z2.x2; x3 = z2; z3 = x2  
 * r_3 = r_0 ^ x0.z0 ^ z0 ^ z0.x0; x3 = x0; z3 = z0  
 * r_3 = r_0 ^ z0;
 *
 */
    void* slice_z = (void*)(tab->slices_z[targ]); 
    void* slice_r = (void*)(tab->phases); 

    for (size_t i = 0; i < tab->n_qubits / 8; i += TABLEAU_SIMD_STRIDE)
    {
        __m256i z = _mm256_load_si256(slice_z + i);
        __m256i r = _mm256_load_si256(slice_r + i);

        _mm256_store_si256(slice_r + i, _mm256_xor_si256(r, z));
    }
}

void tableau_Y(tableau_t* tab, const size_t targ)
{
    /*
     * Y = XZ
     * Z : (r ^= x)
     * X : (r ^= z)
     *
     * r_1 = r_0 ^ x 
     *
     * r_2 = r_0 ^ x ^ z 
     *
     * Y : r ^= x ^ z
     */

    void* slice_x = (void*)(tab->slices_x[targ]); 
    void* slice_z = (void*)(tab->slices_z[targ]); 
    void* slice_r = (void*)(tab->phases); 

    for (size_t i = 0; i < tab->n_qubits / 8; i += TABLEAU_SIMD_STRIDE)
    {
        __m256i x = _mm256_load_si256(slice_x + i);
        __m256i z = _mm256_load_si256(slice_z + i);
        __m256i r = _mm256_load_si256(slice_r + i);

        _mm256_store_si256(slice_r + i, 
            _mm256_xor_si256(r,
                _mm256_xor_si256(x, z)
            )
        );
    }
}

void tableau_HX(tableau_t* tab, const size_t targ)
{
    /*
     * X : (r ^= z)
     * H : (r ^= x.z; x <-> z) 
     *
     * r_1 = r_0 ^ z 
     *
     * r_2 = r_0 ^ z ^ x.z 
     * r_2 = r_0 ^ (~x & z) 
     * Swap x and z
     */

    void* slice_x = (void*)(tab->slices_x[targ]); 
    void* slice_z = (void*)(tab->slices_z[targ]); 
    void* slice_r = (void*)(tab->phases); 

    for (size_t i = 0; i < tab->n_qubits / 8; i += TABLEAU_SIMD_STRIDE)
    {
        __m256i x = _mm256_load_si256(slice_x + i);
        __m256i z = _mm256_load_si256(slice_z + i);
        __m256i r = _mm256_load_si256(slice_r + i);

        _mm256_store_si256(slice_x + i, z);
        _mm256_store_si256(slice_z + i, x);
        _mm256_store_si256(slice_r + i, 
            _mm256_xor_si256(r, 
                _mm256_andnot_si256(x, z)
            )
        );
    }
}

void tableau_SX(tableau_t* tab, const size_t targ)
{
    /*
     * X : (r ^= z)
     * S : (r ^= x.z; z ^= x)
     *
     * r_1 = r_0 ^ z 
     *
     * r_2 = r_0 ^ z ^ x.z 
     * r_2 = r_0 ^ (~x & z) 
     * z_2 = z_0 ^ x
     */
    void* slice_x = (void*)(tab->slices_x[targ]); 
    void* slice_z = (void*)(tab->slices_z[targ]); 
    void* slice_r = (void*)(tab->phases); 

    for (size_t i = 0; i < tab->n_qubits / 8; i += TABLEAU_SIMD_STRIDE)
    {
        __m256i x = _mm256_load_si256(slice_x + i);
        __m256i z = _mm256_load_si256(slice_z + i);
        __m256i r = _mm256_load_si256(slice_r + i);

        _mm256_store_si256(slice_x + i, x);
        _mm256_store_si256(slice_z + i, _mm256_xor_si256(x, z));
        _mm256_store_si256(slice_r + i, 
            _mm256_xor_si256(r, 
                _mm256_andnot_si256(x, z)
            )
        );
    }
}



void tableau_RX(tableau_t* tab, const size_t targ)
{
    /*
     * X : (r ^= z)
     * R : (r ^= x.~z; z ^= x)
     *
     * r_1 = r_0 ^ z 
     *
     * r_2 = r_0 ^ z ^ x.~z 
     * r_2 = r_0 ^ (z | x) 
     * z_2 = z_0 ^ x
     */

    void* slice_x = (void*)(tab->slices_x[targ]); 
    void* slice_z = (void*)(tab->slices_z[targ]); 
    void* slice_r = (void*)(tab->phases); 

    for (size_t i = 0; i < tab->n_qubits / 8; i += TABLEAU_SIMD_STRIDE)
    {
        __m256i x = _mm256_load_si256(slice_x + i);
        __m256i z = _mm256_load_si256(slice_z + i);
        __m256i r = _mm256_load_si256(slice_r + i);

        _mm256_store_si256(slice_x + i, x);
        _mm256_store_si256(slice_z + i, _mm256_xor_si256(x, z));
        _mm256_store_si256(slice_r + i, 
            _mm256_xor_si256(r, 
                _mm256_or_si256(z, x)
            )
        );
    }
}

void tableau_HZ(tableau_t* tab, const size_t targ)
{
    /*
     * Z : (r ^= x)
     * H : (r ^= x.z; x <-> z) 
     *
     * r_1 = r_0 ^ x 
     *
     * r_2 = r_0 ^ x ^ x.z 
     * r_2 = r_0 ^ (x & ~z) 
     * z_2 = x_0
     * x_2 = z_0
     */

    void* slice_x = (void*)(tab->slices_x[targ]); 
    void* slice_z = (void*)(tab->slices_z[targ]); 
    void* slice_r = (void*)(tab->phases); 

    for (size_t i = 0; i < tab->n_qubits / 8; i += TABLEAU_SIMD_STRIDE)
    {
        __m256i x = _mm256_load_si256(slice_x + i);
        __m256i z = _mm256_load_si256(slice_z + i);
        __m256i r = _mm256_load_si256(slice_r + i);

        _mm256_store_si256(slice_x + i, z);
        _mm256_store_si256(slice_z + i, x);
        _mm256_store_si256(slice_r + i, 
            _mm256_xor_si256(r, 
                _mm256_andnot_si256(z, x)
            )
        );
    }
}



void tableau_HY(tableau_t* tab, const size_t targ)
{
    /*
     * Y : r ^= x ^ z
     * H : (r ^= x.z; x <-> z) 
     *
     * r_1 = r_0 ^ x ^ z 
     *
     * r_2 = r_0 ^ x ^ z ^ x.z 
     * r_2 = r_0 ^ (x | z) 
     * z_2 = x_0
     * x_2 = z_0
     */

    void* slice_x = (void*)(tab->slices_x[targ]); 
    void* slice_z = (void*)(tab->slices_z[targ]); 
    void* slice_r = (void*)(tab->phases); 

    for (size_t i = 0; i < tab->n_qubits / 8; i += TABLEAU_SIMD_STRIDE)
    {
        __m256i x = _mm256_load_si256(slice_x + i);
        __m256i z = _mm256_load_si256(slice_z + i);
        __m256i r = _mm256_load_si256(slice_r + i);

        _mm256_store_si256(slice_x + i, z);
        _mm256_store_si256(slice_z + i, x);
        _mm256_store_si256(slice_r + i, 
            _mm256_xor_si256(r, 
                _mm256_xor_si256(z, x)
            )
        );
    }
}


void tableau_SH(tableau_t* tab, const size_t targ)
{
    /*
     * H : (r ^= x.z; x <-> z) 
     * S : (r ^= x.z; z ^= x)
     *
     * r_1 = r_0 ^ x.z 
     * z_1 = x_0
     * x_1 = z_0
     *
     * r_2 = r_1 ^ x_1.z_1
     * r_2 = r_0 ^ x_0.z_0 ^ z_0.x_0
     * r_2 = r_0
     * z_2 ^= x_2 -> x_0 ^= z_0
     * 
     */
    void* slice_x = (void*)(tab->slices_x[targ]); 
    void* slice_z = (void*)(tab->slices_z[targ]); 


    for (size_t i = 0; i < tab->n_qubits / 8; i += TABLEAU_SIMD_STRIDE)
    {
        __m256i x = _mm256_load_si256(slice_x + i);   
        __m256i z = _mm256_load_si256(slice_z + i);

        _mm256_store_si256(slice_x + i, z);
        _mm256_store_si256(
            slice_z + i, 
            _mm256_xor_si256(x, z)
        );
    }
}


void tableau_RH(tableau_t* tab, const size_t targ)
{
    void* slice_x = (void*)(tab->slices_x[targ]); 
    void* slice_z = (void*)(tab->slices_z[targ]); 
    void* slice_r = (void*)(tab->phases); 

    /*
     * H : (r ^= x.z; x <-> z) 
     * R : (r ^= x.~z; z ^= x)
     *
     * r_1 = r_0 ^ x.z 
     * z_1 = x_0
     * x_1 = z_0
     *
     * r_2 = r_1 ^ x_1.~z_1
     * r_2 = r_0 ^ x_0.z_0 ^ z_0.~x_0
     * r_2 = r_0 ^ z_0
     * z_2 ^= x_2 -> x_0 ^= z_0
     *
     */
    // DONE
    for (size_t i = 0; i < tab->n_qubits / 8; i += TABLEAU_SIMD_STRIDE)
    {
        __m256i x = _mm256_load_si256(slice_x + i);   
        __m256i z = _mm256_load_si256(slice_z + i);
        __m256i r = _mm256_load_si256(slice_r + i);

        _mm256_store_si256(slice_x + i, z);
        _mm256_store_si256(slice_z + i, _mm256_xor_si256(x, z));
        _mm256_store_si256(slice_r + i, _mm256_xor_si256(r, z));
    }  
}


void tableau_HS(tableau_t* tab, const size_t targ)
{
    void* slice_x = (void*)(tab->slices_x[targ]); 
    void* slice_z = (void*)(tab->slices_z[targ]); 
    void* slice_r = (void*)(tab->phases); 

    /*
     * S : (r ^= x.z; z ^= x)
     * H : (r ^= x.z; x <-> z) 
     *
     * x_1 = x
     * z_1 = z_0 ^ x
     * r_1 = r_0 ^ x.z 
     *
     * r_2 = r_1 ^ x.z_1
     * r_2 = r_0 ^ x.z ^ x.(x ^ z) 
     * r_2 = r_0 ^ x.z ^ x.~z
     * r_2 = x ^ r_0
     * 
     * z_2 = x_1 = x
     * x_2 = z_1 = z ^ x  
     *
     */
    for (size_t i = 0; i < tab->n_qubits / 8; i += TABLEAU_SIMD_STRIDE)
    {
        __m256i x = _mm256_load_si256(slice_x + i);   
        __m256i z = _mm256_load_si256(slice_z + i);
        __m256i r = _mm256_load_si256(slice_r + i);

        _mm256_store_si256(slice_x + i, _mm256_xor_si256(x, z));
        _mm256_store_si256(slice_z + i, x);
        _mm256_store_si256(slice_r + i, _mm256_xor_si256(r, x));
    }  
}

void tableau_HR(tableau_t* tab, const size_t targ)
{
    void* slice_x = (void*)(tab->slices_x[targ]); 
    void* slice_z = (void*)(tab->slices_z[targ]); 
    /*
     * R : (r ^= x.~z; z ^= x)
     * H : (r ^= x.z; x <-> z) 
     *
     * r_1 = r_0 ^ x.~z 
     * z_1 = z_0 ^ x
     *
     * r_2 = r_1 ^ x.z_1
     * r_2 = r_0 ^ x.~z_0 ^ x.(z_0 ^ x)  
     * x = 0 -> r_0 ^ 0 ^ 0 
     * x = 1 -> r_0 ^ ~z_0 ^ ~z_0   
     * r_2 = r_0
     *
     * z_2 = x_1 = x
     *
     * x_2 = z_1 = z_0 ^ x_0
     *
     */
    // DONE
    for (size_t i = 0; i < tab->n_qubits / 8; i += TABLEAU_SIMD_STRIDE)
    {
        __m256i x = _mm256_load_si256(slice_x + i);   
        __m256i z = _mm256_load_si256(slice_z + i);

        _mm256_store_si256(slice_x + i, _mm256_xor_si256(x, z));
        _mm256_store_si256(slice_z + i, x);
    }  
}

void tableau_HSX(tableau_t* tab, const size_t targ)
{
    /*
     * X : (r ^= z)
     * S : (r ^= x.z; z ^= x)
     * H : (r ^= x.z; x <-> z) 
     *
     * r_1 = r_0 ^ z 
     *
     * r_2 = r_1 ^ x.z_0
     * r_2 = r_0 ^ z_0 ^ x.z_0 
     * r_2 = r_0 ^ ~x.z_0
     * z_2 = z_0 ^ x 
     *
     * r_3 = r_2 ^ x_0.z_2 
     * r_3 = r_0 ^ ~x_0.z_0 ^ x_0.(z_0 ^ x_0)
     *
     * r_3 = r_0 ^ x_0 ^ z_0
     * x_3 = z_2 = x_0 ^ z_0 
     * z_3 = x_2 = x_0
     */

    void* slice_x = (void*)(tab->slices_x[targ]); 
    void* slice_z = (void*)(tab->slices_z[targ]); 
    void* slice_r = (void*)(tab->phases); 

    for (size_t i = 0; i < tab->n_qubits / 8; i += TABLEAU_SIMD_STRIDE)
    {
        __m256i x = _mm256_load_si256(slice_x + i);
        __m256i z = _mm256_load_si256(slice_z + i);
        __m256i r = _mm256_load_si256(slice_r + i);

        _mm256_store_si256(slice_x + i, _mm256_xor_si256(x, z));
        _mm256_store_si256(slice_z + i, x);
        _mm256_store_si256(slice_r + i, 
            _mm256_xor_si256(r, 
                _mm256_xor_si256(z, x)
            )
        );
    }
}

void tableau_HRX(tableau_t* tab, const size_t targ)
{
    /*
     * X : (r ^= z)
     * R : (r ^= x.~z; z ^= x)
     * H : (r ^= x.z; x <-> z) 
     *
     * r_1 = r_0 ^ z 
     *
     * r_2 = r_1 ^ x.~z_0
     * r_2 = r_0 ^ z_0 ^ x.~z_0 
     * r_2 = r_0 ^ (x | z_0)
     * z_2 = z_0 ^ x 
     *
     * r_3 = r_2 ^ x_0.z_2 
     * r_3 = r_0 ^ ~x_0.z_0 ^ x_0.(z_0 ^ x_0)
     * r_3 = r_0 ^ z
     * x_3 = z_2 = x_0 ^ z_0 
     * z_3 = x_2 = x_0
     */

    void* slice_x = (void*)(tab->slices_x[targ]); 
    void* slice_z = (void*)(tab->slices_z[targ]); 
    void* slice_r = (void*)(tab->phases); 

    for (size_t i = 0; i < tab->n_qubits / 8; i += TABLEAU_SIMD_STRIDE)
    {
        __m256i x = _mm256_load_si256(slice_x + i);
        __m256i z = _mm256_load_si256(slice_z + i);
        __m256i r = _mm256_load_si256(slice_r + i);

        _mm256_store_si256(slice_x + i, _mm256_xor_si256(x, z));
        _mm256_store_si256(slice_z + i, x);
        _mm256_store_si256(slice_r + i, _mm256_xor_si256(r, z));
    }
}

void tableau_SHY(tableau_t* tab, const size_t targ)
{
    void* slice_x = (void*)(tab->slices_x[targ]); 
    void* slice_z = (void*)(tab->slices_z[targ]); 
    void* slice_r = (void*)(tab->phases); 

    for (size_t i = 0; i < tab->n_qubits / 8; i += TABLEAU_SIMD_STRIDE)
    {
        __m256i x = _mm256_load_si256(slice_x + i);
        __m256i z = _mm256_load_si256(slice_z + i);
        __m256i r = _mm256_load_si256(slice_r + i);

        _mm256_store_si256(slice_x + i, z);
        _mm256_store_si256(slice_z + i, _mm256_xor_si256(x, z));
        _mm256_store_si256(slice_r + i, _mm256_xor_si256(r, _mm256_xor_si256(x, z)));
    }
}

void tableau_RHY(tableau_t* tab, const size_t targ)
{
    void* slice_x = (void*)(tab->slices_x[targ]); 
    void* slice_z = (void*)(tab->slices_z[targ]); 
    void* slice_r = (void*)(tab->phases); 

    for (size_t i = 0; i < tab->n_qubits / 8; i += TABLEAU_SIMD_STRIDE)
    {
        __m256i x = _mm256_load_si256(slice_x + i);
        __m256i z = _mm256_load_si256(slice_z + i);
        __m256i r = _mm256_load_si256(slice_r + i);

        _mm256_store_si256(slice_x + i, z);
        _mm256_store_si256(slice_z + i, _mm256_xor_si256(x, z));
        _mm256_store_si256(slice_r + i, _mm256_xor_si256(r, x));
    }
}

void tableau_HSH(tableau_t* tab, const size_t targ)
{

    void* slice_x = (void*)(tab->slices_x[targ]); 
    void* slice_z = (void*)(tab->slices_z[targ]); 
    void* slice_r = (void*)(tab->phases); 

    for (size_t i = 0; i < tab->n_qubits / 8; i += TABLEAU_SIMD_STRIDE)
    {
        __m256i x = _mm256_load_si256(slice_x + i);
        __m256i z = _mm256_load_si256(slice_z + i);
        __m256i r = _mm256_load_si256(slice_r + i);

        _mm256_store_si256(slice_x + i, _mm256_xor_si256(x, z));
        _mm256_store_si256(slice_z + i, z);
        _mm256_store_si256(slice_r + i, 
            _mm256_xor_si256(r,
                _mm256_andnot_si256(x, z)
            )
        );
    }
}

void tableau_HRH(tableau_t* tab, const size_t targ)
{
    void* slice_x = (void*)(tab->slices_x[targ]); 
    void* slice_z = (void*)(tab->slices_z[targ]); 
    void* slice_r = (void*)(tab->phases); 

    for (size_t i = 0; i < tab->n_qubits / 8; i += TABLEAU_SIMD_STRIDE)
    {
        __m256i x = _mm256_load_si256(slice_x + i);
        __m256i z = _mm256_load_si256(slice_z + i);
        __m256i r = _mm256_load_si256(slice_r + i);

        _mm256_store_si256(slice_x + i, _mm256_xor_si256(x, z));
        _mm256_store_si256(slice_z + i, z);
        _mm256_store_si256(slice_r + i, 
            _mm256_xor_si256(r,
                _mm256_and_si256(x, z)
            )
        );
    }
}


void tableau_RHS(tableau_t* tab, const size_t targ)
{
    void* slice_x = (void*)(tab->slices_x[targ]); 
    void* slice_z = (void*)(tab->slices_z[targ]); 
    void* slice_r = (void*)(tab->phases); 

    for (size_t i = 0; i < tab->n_qubits / 8; i += TABLEAU_SIMD_STRIDE)
    {
        __m256i x = _mm256_load_si256(slice_x + i);
        __m256i z = _mm256_load_si256(slice_z + i);
        __m256i r = _mm256_load_si256(slice_r + i);

        _mm256_store_si256(slice_x + i, _mm256_xor_si256(x, z));
        _mm256_store_si256(slice_z + i, z);
        _mm256_store_si256(slice_r + i, 
            _mm256_xor_si256(r,
                _mm256_or_si256(x, z)
            )
        );
    }
}


void tableau_SHR(tableau_t* tab, const size_t targ)
{
    void* slice_x = (void*)(tab->slices_x[targ]); 
    void* slice_z = (void*)(tab->slices_z[targ]); 
    void* slice_r = (void*)(tab->phases); 

    for (size_t i = 0; i < tab->n_qubits / 8; i += TABLEAU_SIMD_STRIDE)
    {
        __m256i x = _mm256_load_si256(slice_x + i);
        __m256i z = _mm256_load_si256(slice_z + i);
        __m256i r = _mm256_load_si256(slice_r + i);

        _mm256_store_si256(slice_x + i, _mm256_xor_si256(x, z));
        _mm256_store_si256(slice_z + i, z);
        _mm256_store_si256(slice_r + i, 
            _mm256_xor_si256(r,
                _mm256_andnot_si256(z, x)
            )
        );
    }
}

void tableau_CNOT(tableau_t* tab, const size_t ctrl, const size_t targ)
{
    /*
     * CNOT a, b: ( 
     *     r ^= x_a & z_b & (1 ^ x_b ^ z_a);
     *     x_b ^= x_a;
     *     z_a ^= z_b) 
     *
     */ 

    void* ctrl_slice_x = (void*)(tab->slices_x[ctrl]); 
    void* ctrl_slice_z = (void*)(tab->slices_z[ctrl]); 
    void* targ_slice_x = (void*)(tab->slices_x[targ]); 
    void* targ_slice_z = (void*)(tab->slices_z[targ]); 
    void* slice_r = (void*)(tab->phases); 

    for (size_t i = 0; i < tab->n_qubits / 8; i += TABLEAU_SIMD_STRIDE)
    {
        __m256i ctrl_x = _mm256_load_si256(ctrl_slice_x + i);
        __m256i ctrl_z = _mm256_load_si256(ctrl_slice_z + i);
        __m256i targ_x = _mm256_load_si256(targ_slice_x + i);
        __m256i targ_z = _mm256_load_si256(targ_slice_z + i);
        __m256i r = _mm256_load_si256(slice_r + i);

        _mm256_store_si256(targ_slice_x + i, _mm256_xor_si256(ctrl_x, targ_x));
        _mm256_store_si256(ctrl_slice_z + i, _mm256_xor_si256(ctrl_z, targ_z));
        _mm256_store_si256(slice_r + i, 
            _mm256_xor_si256(r,
                _mm256_andnot_si256(
                    _mm256_xor_si256(targ_x, ctrl_z), 
                    _mm256_and_si256(ctrl_x, targ_z) 
                )
            )
        );
    }
}

void tableau_CZ(tableau_t* tab, const size_t ctrl, const size_t targ)
{
    /* CZ = H_b CNOT H_b 
     * H : (r ^= x.z; x <-> z) 
     * CNOT a, b: ( 
     *     r ^= x_a & z_b & ~(x_b ^ z_a);
     *     x_b ^= x_a;
     *     z_a ^= z_b) 
     *
     * H:
     * r_1 = r_0 ^ x_b.z_b 
     * x_a1 = x_a0
     * x_b1 = z_b0 
     * z_a1 = z_a0
     * z_b1 = x_b0 
     *
     * CNOT:
     * r_2 = r_1 ^ (x_a0 & z_b1 & ~(x_b1 ^ z_a0))
     * r_2 = r_0 ^ (x_b0 & z.b0) ^ (x_a0 & x_b0 & ~(z_b0 ^ z_a0))
     * x_a2 = x_a1
     * x_a2 = x_a0 
     * x_b2 = x_b1 ^ x_a1
     * x_b2 = z_b0 ^ x_a0
     * z_a2 = z_a1 ^ z_b1 
     * z_a2 = z_a0 ^ x_b0 
     * z_b2 = z_b1
     * z_b2 = x_b0
     *
     * H: 
     * r_3 = r_2 ^ (x_b2 & z_b2)  
     * r_3 = r_0 ^ (x_b0 & z_b0) ^ (x_a0 & x_b0 & ~(z_b0 ^ z_a0)) ^ ((z_b0 ^ x_a0) & x_b0); 
     * x_a3 = x_a2 = x_a0 
     * x_b3 = z_b2 = x_b0 
     * z_a3 = z_a2 = z_a0 ^ x_b0 
     * z_b3 = x_b2 = z_b0 ^ x_a0 
     *
     */ 

    void* ctrl_slice_x = (void*)(tab->slices_x[ctrl]); 
    void* ctrl_slice_z = (void*)(tab->slices_z[ctrl]); 
    void* targ_slice_x = (void*)(tab->slices_x[targ]); 
    void* targ_slice_z = (void*)(tab->slices_z[targ]); 
    void* slice_r = (void*)(tab->phases); 

    for (size_t i = 0; i < tab->n_qubits / 8; i += TABLEAU_SIMD_STRIDE)
    {
        __m256i ctrl_x = _mm256_load_si256(ctrl_slice_x + i);
        __m256i ctrl_z = _mm256_load_si256(ctrl_slice_z + i);
        __m256i targ_x = _mm256_load_si256(targ_slice_x + i);
        __m256i targ_z = _mm256_load_si256(targ_slice_z + i);
        __m256i r = _mm256_load_si256(slice_r + i);

        _mm256_store_si256(targ_slice_z + i, _mm256_xor_si256(ctrl_x, targ_z));
        _mm256_store_si256(ctrl_slice_z + i, _mm256_xor_si256(ctrl_z, targ_x));

        __m256i x_and_x = _mm256_and_si256(ctrl_x, targ_x);

        _mm256_store_si256(slice_r + i, 
            _mm256_xor_si256(r,
                _mm256_xor_si256(
                    _mm256_and_si256(x_and_x, ctrl_z), 
                    _mm256_and_si256(x_and_x, targ_z) 
                )
            )
        );
    }
}
