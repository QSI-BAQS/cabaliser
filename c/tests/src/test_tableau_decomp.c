#include <assert.h>

#define INSTRUCTIONS_TABLE

#include "widget.h"
#include "tableau_operations.h"
#include "input_stream.h"
#include "instructions.h"


instruction_stream_u* create_instruction_stream(const size_t n_qubits, const size_t n_gates)
{

    instruction_stream_u* inst = malloc(n_gates * sizeof(instruction_stream_u));  

    for (size_t i = 0; i < n_gates / 4; i++)
    {
        uint8_t opcode = LOCAL_CLIFFORD_MASK | (rand() % N_LOCAL_CLIFFORD_INSTRUCTIONS);
        size_t arg = rand() % n_qubits; 
        inst[i].single.opcode = opcode;
        inst[i].single.arg = arg;
    }

    for (size_t i = n_gates / 4; i < n_gates / 2 ; i++)
    {
        uint8_t opcode = NON_LOCAL_CLIFFORD_MASK | (rand() % N_NON_LOCAL_CLIFFORD_INSTRUCTIONS);
        size_t ctrl = rand() % n_qubits; 
        size_t targ;

        while ((targ = (rand() % n_qubits)) == ctrl){}; 

        inst[i].multi.opcode = opcode;
        inst[i].multi.ctrl = ctrl;
        inst[i].multi.targ = targ;
    }

    for (size_t i = n_gates / 2; i < 3 * (n_gates / 4); i++)
    {
        uint8_t opcode = LOCAL_CLIFFORD_MASK | (rand() % N_LOCAL_CLIFFORD_INSTRUCTIONS);
        size_t arg = rand() % n_qubits; 
        inst[i].single.opcode = opcode;
        inst[i].single.arg = arg;
    }

    for (size_t i = n_gates - (3 * (n_gates / 4)); i < n_gates; i++)
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


void test_identity(void)
{
    const size_t n_qubits = 8;
    widget_t* wid = widget_random_create(n_qubits, n_qubits);

    for (size_t i = 0; i < n_qubits; i++)
    {
        memset(wid->tableau->slices_x[i], 0x00, 64);
        wid->tableau->slices_z[i][0] |= (1 << i); 
    }

    tableau_remove_zero_X_columns(wid->tableau, wid->queue); 

    tableau_transpose(wid->tableau);

    for (size_t i = 0; i < n_qubits; i++)
    {
        tableau_X_diag_element(wid->tableau, wid->queue, i);
    }


    for (size_t i = 0; i < n_qubits; i++)//n_qubits; i++)
    {
        if (!__inline_slice_get_bit(wid->tableau->slices_x[i], i))
        {
            tableau_X_diag_element(wid->tableau, wid->queue, i);
        }
        tableau_X_diag_col(wid->tableau, i);
    }

    for (size_t i = 0; i < n_qubits; i++)//n_qubits; i++)
    {
        if (__inline_slice_get_bit(wid->tableau->slices_z[i], i))
        {
            // TODO transverse phase
            __inline_slice_set_bit(wid->tableau->slices_z[i], i, 0);
        }
    }

    // Transverse Z to set phases to 0
    for (size_t i = 0; i < n_qubits; i++)//n_qubits; i++)
    {
        if (__inline_slice_get_bit(wid->tableau->phases, i))
        {
            // TODO transverse Z 
            __inline_slice_set_bit(wid->tableau->phases, i, 0);
        }
    }

    for (size_t i = 0; i < n_qubits; i++)//n_qubits; i++)
    {
        if (__inline_slice_get_bit(wid->tableau->slices_z[i], i))
        {
            // TODO transverse phase
            __inline_slice_set_bit(wid->tableau->slices_z[i], i, 0);
        }
        tableau_X_diag_col(wid->tableau, i);
    }


    for (size_t i = 0; i < n_qubits; i++)//n_qubits; i++)
    {
        assert(0 == __inline_slice_get_bit(wid->tableau->slices_z[i], i));
        assert(1 == __inline_slice_get_bit(wid->tableau->slices_x[i], i));
        assert(i == tableau_ctz(wid->tableau->slices_x[i], wid->tableau->slice_len));
    }


    widget_destroy(wid);
    return;
}


int main()
{
    for (size_t i = 0; i < 100; i++)
    {
        srand(2 * i);
        test_identity();
    }

    return 0;
}
