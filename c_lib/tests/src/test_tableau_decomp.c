#include <assert.h>

#define INSTRUCTIONS_TABLE

#include <stdbool.h>

#include "widget.h"
#include "tableau_operations.h"
#include "simd_gaussian_elimination.h"


#include "tableau.h"
#include "test_tableau.h"

#include "input_stream.h"
#include "instructions.h"

/*
 * Tests correctness of load and store block operations on a fresh tableau
 * Blocks are loaded, asserted then flipped
 * Test will fail to catch a ctzll on the first bit of a Z block, or the second 
 * bit for the first column.     
 * This is covered by checking all elements in the block
 */
void test_load_block(const size_t n_qubits)
{
    // Initial state of tableau is diagonal in X, empty in Z
    tableau_t* tab = tableau_create(n_qubits);

    uint64_t block[64];
    for (size_t i = 0; i < n_qubits; i += 64)
    {
        // Z should be diagonal
        decomp_load_block(block, tab->slices_z[0], tab->slice_len, i, i);
        for (size_t k = 0; k < 64; k++)
        {
            assert(k == __builtin_ctzll(block[k]));
            block[k] = ~block[k];
        }
        decomp_store_block(block, tab->slices_z[0], tab->slice_len, i, i);
     
        // X should be empty 
        decomp_load_block(block, tab->slices_x[0], tab->slice_len, i, i);
        for (size_t k = 0; k < 64; k++)
        {
            assert(0ull == block[k]);
            block[k] = ~block[k];
        }
        decomp_store_block(block, tab->slices_x[0], tab->slice_len, i, i);

    } 

    for (size_t i = 0; i < n_qubits; i += 64)
    {
        for (size_t j  = 0; j < n_qubits; j += 64)
        {
            if (i != j)
            {
                // Z should be diagonal
                decomp_load_block(block, tab->slices_z[0], tab->slice_len, i, j);
                for (size_t k = 0; k < 64; k++)
                {
                    assert(0ull == block[k]);
                    block[k] = ~block[k];
                }
                decomp_store_block(block, tab->slices_z[0], tab->slice_len, i, j);
             
                // X should be empty 
                decomp_load_block(block, tab->slices_x[0], tab->slice_len, i, j);
                for (size_t k = 0; k < 64; k++)
                {
                    assert(0ull == block[k]);
                    block[k] = ~block[k];
                }
                decomp_store_block(block, tab->slices_x[0], tab->slice_len, i, j);

            }
        }
    } 

    tableau_destroy(tab);
}


instruction_stream_u* create_sample_instruction_stream()
{
    instruction_stream_u* inst = malloc(3 * sizeof(instruction_stream_u));  
    inst[0].single.opcode = _H_;
    inst[0].single.arg = 0;

    inst[1].multi.opcode = _CNOT_;
    inst[1].multi.ctrl = 0;
    inst[1].multi.targ = 1;

    inst[2].multi.opcode = _CNOT_;
    inst[2].multi.ctrl = 1;
    inst[2].multi.targ = 2;    
    return inst;
}

instruction_stream_u* create_instruction_stream_local(const size_t n_qubits, const size_t n_gates)
{

    instruction_stream_u* inst = malloc(n_gates * sizeof(instruction_stream_u));  

    for (size_t i = 0; i < n_gates; i++)
    {
        uint8_t opcode = LOCAL_CLIFFORD_MASK | (rand() % N_LOCAL_CLIFFORD_INSTRUCTIONS);
        size_t arg = rand() % n_qubits; 
        inst[i].single.opcode = opcode;
        inst[i].single.arg = arg;
    }

    return inst;
}

instruction_stream_u* create_instruction_stream_non_local(const size_t n_qubits, const size_t n_gates)
{

    instruction_stream_u* inst = malloc(n_gates * sizeof(instruction_stream_u));  

    for (size_t i = 0; i < n_gates; i++)
    {
        uint8_t opcode = NON_LOCAL_CLIFFORD_MASK | (rand() %  2);


        size_t ctrl = rand() % n_qubits; 
        size_t targ = rand() % n_qubits;

        while ((targ = (rand() % n_qubits)) == ctrl){}; 

        inst[i].multi.opcode = opcode;
        inst[i].multi.ctrl = ctrl;
        inst[i].multi.targ = targ;
    }

    return inst;
}

instruction_stream_u* create_instruction_stream_in_block(const size_t n_qubits, const size_t n_gates)
{

    instruction_stream_u* inst = malloc(n_gates * sizeof(instruction_stream_u));  

    size_t blocks = n_qubits / 64 + !!(n_qubits % 64);
    if ((n_qubits % 64) == 1)
    { 
        blocks--;
    }

    for (size_t i = 0; i < n_gates; i++)
    {
        uint8_t opcode = NON_LOCAL_CLIFFORD_MASK | (rand() %  2);


        size_t ctrl = -1ull; 
        size_t targ = -1ull;

        while ((n_qubits <= ctrl) || n_qubits <= targ)
        {

            ctrl = rand() % 64;
            while (ctrl == (targ = rand() % 64)){};

            size_t block = rand() % blocks;
            ctrl += block * 64;
            targ += block * 64;
        }

        inst[i].multi.opcode = opcode;
        inst[i].multi.ctrl = ctrl;
        inst[i].multi.targ = targ;
    }

    return inst;
}


instruction_stream_u* create_instruction_stream(const size_t n_qubits, const size_t n_gates)
{

    instruction_stream_u* inst = malloc(n_gates * sizeof(instruction_stream_u));  

    for (size_t i = 0; i < n_gates / 2; i++)
    {
        uint8_t opcode = LOCAL_CLIFFORD_MASK | (rand() % N_LOCAL_CLIFFORD_INSTRUCTIONS);
        size_t arg = rand() % n_qubits; 
        inst[i].single.opcode = opcode;
        inst[i].single.arg = arg;
    }

    for (size_t i = n_gates / 2; i < n_gates; i++)
    {
        uint8_t opcode = NON_LOCAL_CLIFFORD_MASK | (rand() % N_NON_LOCAL_CLIFFORD_INSTRUCTIONS);
        size_t ctrl = rand() % n_qubits; 
        size_t targ;

        while ((targ = (rand() % n_qubits)) == ctrl){}; 

        inst[i].multi.opcode = opcode;
        inst[i].multi.ctrl = ctrl;
        inst[i].multi.targ = targ;
    }

    return inst;
}

/*
 * Creates and applies single qubit cliffords
 */
widget_t* widget_random_create_local(size_t n_qubits, size_t n_gates)
{
    widget_t* wid = widget_create(n_qubits, n_qubits);

    instruction_stream_u* stream = create_instruction_stream_local(n_qubits, n_gates);
    parse_instruction_block(wid, stream, n_gates);
    free(stream); 
    return wid; 
}

/*
 * Creates and applies two qubit cliffords across all n qubits
 */
widget_t* widget_random_create_non_local(size_t n_qubits, size_t n_gates)
{
    widget_t* wid = widget_create(n_qubits, n_qubits);

    instruction_stream_u* stream = create_instruction_stream_non_local(n_qubits, n_gates);
    parse_instruction_block(wid, stream, n_gates);
    free(stream); 
    return wid; 
}

/*
 * Creates and applies two qubit cliffords in blocks of 64
 */
widget_t* widget_random_create_in_block(size_t n_qubits, size_t n_gates)
{
    widget_t* wid = widget_create(n_qubits, n_qubits);

    instruction_stream_u* stream = create_instruction_stream_in_block(n_qubits, n_gates);
    parse_instruction_block(wid, stream, n_gates);
    free(stream); 
    return wid; 
}

widget_t* widget_random_create(size_t n_qubits, size_t n_gates)
{
    widget_t* wid = widget_create(n_qubits, n_qubits);

    instruction_stream_u* stream = create_instruction_stream(n_qubits, n_gates);
    parse_instruction_block(wid, stream, n_gates);
    free(stream); 
    return wid; 
} 

widget_t* widget_hadamard_create(size_t n_qubits)
{
    widget_t* wid = widget_create(n_qubits, n_qubits);

    instruction_stream_u* inst = malloc(n_qubits * sizeof(instruction_stream_u));  
    for (size_t i = 0; i < n_qubits; i++)
    {
        inst[i].single.opcode = _H_;
        inst[i].single.arg = 0;
    }

    parse_instruction_block(wid, inst, n_qubits);
    apply_local_cliffords(wid);
    free(inst); 
    return wid; 
}


widget_t* widget_sample_create()
{
    widget_t* wid = widget_create(3, 3);

    instruction_stream_u* stream = create_sample_instruction_stream();
    parse_instruction_block(wid, stream, 3);
    free(stream); 
    
    return wid;
}




static inline 
widget_t* __test_block_diag_preamble(size_t n_qubits, widget_t* wid, bool clear_x)
{
    if (clear_x)
    {
        tableau_remove_zero_X_columns(wid->tableau, wid->queue);
    }
    tableau_transpose(wid->tableau);

    widget_t* cpy = widget_create(n_qubits, n_qubits);
    tableau_destroy(cpy->tableau);

    cpy->tableau = tableau_copy(wid->tableau); 
    return cpy;
}

static inline
void __test_block_diag(const size_t n_qubits, widget_t* wid, widget_t* cpy)
{

    tableau_elim_upper(wid);
    simd_tableau_elim_upper(cpy);

    for (size_t i = 0; i < n_qubits; i++)
    {

        assert(1 == __inline_slice_get_bit(wid->tableau->slices_x[i], i));
        assert(1 == __inline_slice_get_bit(cpy->tableau->slices_x[i], i));

        for (size_t j = 0; j < i; j++)
        {
            assert(0 == __inline_slice_get_bit(wid->tableau->slices_x[i], j));
            assert(0 == __inline_slice_get_bit(cpy->tableau->slices_x[i], j));
        }
    } 

    //tableau_elim_lower(wid);
    //tableau_elim_lower(cpy);

    //for (size_t i = 0; i < n_qubits; i++)
    //{

    //    assert(1 == __inline_slice_get_bit(wid->tableau->slices_x[i], i));
    //    assert(1 == __inline_slice_get_bit(cpy->tableau->slices_x[i], i));

    //    for (size_t j = i + 1; j < n_qubits; j++)
    //    {
    //        assert(0 == __inline_slice_get_bit(wid->tableau->slices_x[i], j));
    //        assert(0 == __inline_slice_get_bit(cpy->tableau->slices_x[i], j));
    //    }
    //} 


    return;
}

void test_block_diag(const size_t n_qubits, widget_t* wid)
{
    widget_t* cpy = __test_block_diag_preamble(n_qubits, wid, true);
    __test_block_diag(n_qubits, wid, cpy); 
    widget_destroy(cpy);
}

void test_block_diag_hadamard(const size_t n_qubits, widget_t* wid)
{
    widget_t* cpy = __test_block_diag_preamble(n_qubits, wid, false);
    __test_block_diag(n_qubits, wid, cpy); 
    widget_destroy(cpy);
}



void test_idx_swap(size_t n_qubits)
{
    widget_t* wid = widget_random_create(n_qubits, n_qubits);

    void* stage_x = malloc(n_qubits / 8 + 1);
    void* stage_z = malloc(n_qubits / 8 + 1);

    for (size_t i = 0; i < n_qubits; i++)
    {
        for (size_t j = 0; j < i; j++)
        {
            memcpy(stage_x, wid->tableau->slices_x[i], n_qubits / 8 + 1);
            memcpy(stage_z, wid->tableau->slices_z[i], n_qubits / 8 + 1);

            tableau_idx_swap_transverse(wid->tableau, i, j);
            assert(0 == memcmp(stage_x, wid->tableau->slices_x[j], n_qubits / 8 + 1));
            assert(0 == memcmp(stage_z, wid->tableau->slices_z[j], n_qubits / 8 + 1));

            simd_tableau_idx_swap_transverse(wid->tableau, i, j);
            assert(0 == memcmp(stage_x, wid->tableau->slices_x[i], n_qubits / 8 + 1));
            assert(0 == memcmp(stage_z, wid->tableau->slices_z[i], n_qubits / 8 + 1));
        }
    } 

    return;
}


void test_ghz(void)
{
    const size_t n_qubits = 3;
    widget_t* wid = widget_sample_create();

    widget_decompose(wid);    

    for (size_t i = 0; i < n_qubits; i++)//n_qubits; i++)
    {
        assert(0 == __inline_slice_get_bit(wid->tableau->slices_z[i], i));
        assert(1 == __inline_slice_get_bit(wid->tableau->slices_x[i], i));
        assert(i == tableau_ctz(wid->tableau->slices_x[i], wid->tableau->slice_len));
    }

    widget_destroy(wid);
    return;
}


void test_random(const size_t n_qubits)
{
    widget_t* wid = widget_random_create(n_qubits, n_qubits * 10);


    widget_decompose(wid);    


    for (size_t i = 0; i < n_qubits; i++)//n_qubits; i++)
    {
        assert(0 == __inline_slice_get_bit(wid->tableau->slices_z[i], i));
        assert(1 == __inline_slice_get_bit(wid->tableau->slices_x[i], i));
        assert(i == tableau_ctz(wid->tableau->slices_x[i], wid->tableau->slice_len));
    }

    for (size_t i = 0; i < wid->tableau->slice_len; i++)
    {
        assert(0 == wid->tableau->phases[i]);
    }

    widget_destroy(wid);
    return;
}

int main()
{

    // Test decomposition of in-block two-qubit Cliffords only 
    // Single block tests
//    for (size_t i = 64; i <= 256 ; i += 64)
//    {
//        widget_t* wid = widget_random_create_in_block(i, i * i);
//        apply_local_cliffords(wid);
//        test_block_diag(i, wid);
//        widget_destroy(wid);
//    }

//    for (size_t i = 128; i <= 128; i += 64)
//    {
//        test_load_block(i);
//    }

    for (size_t i = 128; i <= 128; i += 64)
    {
        widget_t* wid = widget_hadamard_create(i);
        apply_local_cliffords(wid);
        test_block_diag_hadamard(i, wid);
        tableau_print(wid->tableau);  
        widget_destroy(wid);
    }

    // Test decomposition of single qubit Cliffords only 
    // Single block tests
//    for (size_t i = 64; i <= 256 ; i += 64)
//    {
//        widget_t* wid = widget_random_create_local(i, i * i);
//        apply_local_cliffords(wid);
//        test_block_diag(i, wid);
//        widget_destroy(wid);
//    }

//    // Larger and more irregular tableau sizes
//    for (size_t i = 10; i < 1280; i += 17)
//    {
//        widget_t* wid = widget_random_create_local(i, i * i);
//        apply_local_cliffords(wid);
//        test_block_diag(i, wid);
//        widget_destroy(wid);
//    }

    // Solved by application of local cliffords

//    for (size_t i = 10; i < 1280; i+=10)
//    {
//      test_block_diag_cliffords(i);
//    }

    // Solveable by hadamard after local elim fails
//    test_block_diag_hadamard(8);
//    test_block_diag_hadamard(17);
//    test_block_diag_hadamard(64);

    // Solveable by non-local elimination
//    for (size_t i = 10; i < 1280; i+=10)
//    {
//      test_block_diag_hadamard(i);
//    }

    // Solved by local elimination 
//    test_block_diag_local(8);
//    test_block_diag_local(17);
//    test_block_diag_local(64);
//    test_block_diag_non_local(128);

//
//    for (size_t i = 10; i < 1280; i+=10)
//    {
//      test_block_diag_local(i);
//    }


    // Testing in-block elimination
//    for (size_t i = 10; i < 1280; i+=10)
//    {
//      test_block_diag_non_local(i);
//    }

//    test_block_diag_non_local(128);
//    test_block_diag_non_local(256);
//
//    test_block_diag_non_local(128);
//    test_block_diag_non_local(256);

//    test_block_diag_non_local(128);
//    test_block_diag_non_local(128);
//    test_block_diag_non_local(128);
//    test_block_diag_non_local(128);



    //for (size_t i = 10; i < 128; i+=10)
    //{
    //    test_idx_swap(i);
    //}
    //
    //test_ghz();
    //for (size_t i = 0; i < 100; i++)
    //{
    //    srand(i);
    //    test_random(8);
    //}

    //for (size_t i = 10; i < 100; i+= 10)
    //{
    //    srand(i);
    //    test_random(i * 64);
    //}

    return 0;
}
