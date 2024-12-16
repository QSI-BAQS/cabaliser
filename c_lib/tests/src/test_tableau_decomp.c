#include <assert.h>

#define INSTRUCTIONS_TABLE

#include "widget.h"
#include "tableau_operations.h"
#include "simd_gaussian_elimination.h"


#include "tableau.h"
#include "test_tableau.h"

#include "input_stream.h"
#include "instructions.h"



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


widget_t* widget_random_create(size_t n_qubits, size_t n_gates)
{
    widget_t* wid = widget_create(n_qubits, n_qubits);

    instruction_stream_u* stream = create_instruction_stream(n_qubits, n_gates);
    parse_instruction_block(wid, stream, n_gates);
    free(stream); 
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


void test_block_diag(size_t n_qubits)
{
    widget_t* wid = widget_random_create(n_qubits, n_qubits * n_qubits);
    widget_t* cpy = widget_create(n_qubits, n_qubits);
    tableau_destroy(cpy->tableau);

    tableau_remove_zero_X_columns(wid->tableau, wid->queue);
    tableau_transpose(wid->tableau);

    cpy->tableau = tableau_copy(wid->tableau); 

    tableau_elim_upper(wid);
    simd_tableau_elim_upper(cpy);

    for (size_t i = 0; i < n_qubits; i++)
    {

        assert(1 == __inline_slice_get_bit(wid->tableau->slices_x[i], i));
        assert(1 == __inline_slice_get_bit(cpy->tableau->slices_x[i], i));

        for (size_t j = i + 1; j < i; j++)
        {
            assert(0 == __inline_slice_get_bit(wid->tableau->slices_x[i], i));
            assert(0 == __inline_slice_get_bit(cpy->tableau->slices_x[i], i));
        }
    } 

    return;
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

    test_block_diag(8);

    for (size_t i = 10; i < 128; i+=10)
    {
        test_idx_swap(i);
    }
    
    test_ghz();
    for (size_t i = 0; i < 100; i++)
    {
        srand(i);
        test_random(8);
    }

    for (size_t i = 10; i < 100; i+= 10)
    {
        srand(i);
        test_random(i * 64);
    }

    return 0;
}
