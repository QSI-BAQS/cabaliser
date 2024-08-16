#include <assert.h>

#define INSTRUCTIONS_TABLE

#include "widget.h"
#include "tableau_operations.h"
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

    widget_destroy(wid);
    return;
}

int main()
{
    
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
