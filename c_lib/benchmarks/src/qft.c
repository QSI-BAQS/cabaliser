#include <assert.h>

#define INSTRUCTIONS_TABLE

#include "widget.h"
#include "tableau_operations.h"
#include "input_stream.h"
#include "instructions.h"


instruction_stream_u* create_instruction_stream_qft(const size_t n_qubits, size_t* n_gates_ret)
{
    size_t n_gates = n_qubits + 3 * (n_qubits  + 1) * n_qubits;

    instruction_stream_u* inst = malloc(n_gates * sizeof(instruction_stream_u));  
    size_t curr_instruction = 0;

    for (size_t i = 0; i < n_qubits; i++)
    {
        inst[curr_instruction].single.opcode = _H_;
        inst[curr_instruction].single.arg = i;
        curr_instruction++;
        for (size_t j = i + 1; j < n_qubits; j++)
        {

            inst[curr_instruction].rz.opcode = _RZ_;
            inst[curr_instruction].rz.arg = i;
            inst[curr_instruction].rz.tag = j;
            curr_instruction++;

            inst[curr_instruction].rz.opcode = _RZ_;
            inst[curr_instruction].rz.arg = j;
            inst[curr_instruction].rz.tag = j;
            curr_instruction++;

            inst[curr_instruction].multi.opcode = _CNOT_;
            inst[curr_instruction].multi.ctrl = j;
            inst[curr_instruction].multi.targ = i;
            curr_instruction++;

            inst[curr_instruction].rz.opcode = _RZ_;
            inst[curr_instruction].rz.arg = i;
            inst[curr_instruction].rz.tag = j;
            curr_instruction++;

            inst[curr_instruction].multi.opcode = _CNOT_;
            inst[curr_instruction].multi.ctrl = j;
            inst[curr_instruction].multi.targ = i;
            curr_instruction++;
        }
    }

    *n_gates_ret = curr_instruction;

    return inst;
}


widget_t* widget_qft_create(size_t n_qubits)
{
    size_t n_gates = 0;
    size_t max_tableau = 2 * n_qubits + 1.5 * n_qubits * n_qubits; 

    instruction_stream_u* stream = create_instruction_stream_qft(n_qubits, &n_gates);

    widget_t* wid = widget_create(n_qubits, max_tableau);
    parse_instruction_block(wid, stream, n_gates);
    free(stream); 
    return wid; 
} 



void qft_benchmark(const size_t n_qubits)
{
    widget_t* wid = widget_qft_create(n_qubits);

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

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printf("Insufficient parameters, requires <n_qubits>\n");
	return 0;
    }

    size_t tableau_size = atoi(argv[1]);

    qft_benchmark(tableau_size);

    return 0;
}
