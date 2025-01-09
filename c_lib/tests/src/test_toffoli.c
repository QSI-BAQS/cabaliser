#include <assert.h>

#define INSTRUCTIONS_TABLE

#include "widget.h"
#include "tableau_operations.h"
#include "input_stream.h"
#include "instructions.h"


#define _T_ (1)
#define _Tdag_ (2)

instruction_stream_u* create_toffoli()
{
    instruction_stream_u* inst = malloc(14 * sizeof(instruction_stream_u));  

    inst[0].single.opcode = _H_;
    inst[0].single.arg = 2;

    // Sets a 1 between 1 and 2
    inst[1].multi.opcode = _CNOT_;
    inst[1].multi.ctrl = 1;
    inst[1].multi.targ = 2;

    // Maps qubit 2 to 3
    // Qubit 2 terminates on _Tdag_
    inst[2].rz.opcode = _RZ_;
    inst[2].rz.arg = 2;
    inst[2].rz.tag = _Tdag_;

    // Sets a 1 between 0 and 3 
    inst[3].multi.opcode = _CNOT_;
    inst[3].multi.ctrl = 0;
    inst[3].multi.targ = 2;    

    // Maps qubit 2 to 4
    // Qubit 3 terminates on _T_
    inst[4].rz.opcode = _RZ_;
    inst[4].rz.arg = 2;
    inst[4].rz.tag = _T_;

    // Sets a 1 between 0 and 4
    inst[5].multi.opcode = _CNOT_;
    inst[5].multi.ctrl = 1;
    inst[5].multi.targ = 2;    

    // Maps qubit 2 to 5
    // Qubit 4 terminates on _Tdag_
    inst[6].rz.opcode = _RZ_;
    inst[6].rz.arg = 2;
    inst[6].rz.tag = _Tdag_;

    inst[7].multi.opcode = _CNOT_;
    inst[7].multi.ctrl = 0;
    inst[7].multi.targ = 2;    

    // Maps qubit 2 to 6
    // Qubit 5 terminates on _T_
    inst[8].rz.opcode = _RZ_;
    inst[8].rz.arg = 2;
    inst[8].rz.tag = _T_;

    inst[9].multi.opcode = _CNOT_;
    inst[9].multi.ctrl = 0;
    inst[9].multi.targ = 1;    

    // Maps qubit 0 to 7
    // Qubit 0 terminates on _T_
    inst[10].rz.opcode = _RZ_;
    inst[10].rz.arg = 0;
    inst[10].rz.tag = _T_;

    // Maps qubit 1 to 8
    // Qubit 1 terminates on _Tdag_
    inst[11].rz.opcode = _RZ_;
    inst[11].rz.arg = 1;
    inst[11].rz.tag = _Tdag_;

    inst[12].multi.opcode = _CNOT_;
    inst[12].multi.ctrl = 0;
    inst[12].multi.targ = 1;    

    inst[13].single.opcode = _H_;
    inst[13].single.arg = 2;

    return inst;
}


void test_toffoli()
{
    // 3 initial qubits, widget size of 20
    widget_t* wid = widget_create(3, 9);

    instruction_stream_u* stream = create_toffoli();

    parse_instruction_block(wid, stream, 14);
    free(stream); 

    widget_decompose(wid);    


    for (size_t i = 0; i < wid->n_qubits; i++)//n_qubits; i++)
    {
        assert(0 == __inline_slice_get_bit(wid->tableau->slices_z[i], i));
        assert(1 == __inline_slice_get_bit(wid->tableau->slices_x[i], i));
        assert(i == tableau_ctz(wid->tableau->slices_x[i], wid->tableau->slice_len));
    }

    for (size_t i = 0; i < wid->tableau->n_qubits; i += 64)
    {
        assert(0 == *(wid->tableau->phases + (i / 64)));
    }

    // For arguments about these numbers see the toffoli function above
    assert(7 == wid->q_map[0]); 
    assert(8 == wid->q_map[1]); 
    assert(6 == wid->q_map[2]); 

    assert(_T_ == wid->queue->non_cliffords[0]);
    assert(_Tdag_ == wid->queue->non_cliffords[1]);
    assert(_Tdag_ == wid->queue->non_cliffords[2]);
    assert(_T_ == wid->queue->non_cliffords[3]);
    assert(_Tdag_ == wid->queue->non_cliffords[4]);
    assert(_T_ == wid->queue->non_cliffords[5]);

    // Final tableau state should have the third qubit connected to the next four 
    assert(0 == wid->tableau->slices_z[0][0]);
    assert(0 == wid->tableau->slices_z[0][0]);
    assert(120 == wid->tableau->slices_z[2][0]);
    assert(4 == wid->tableau->slices_z[3][0]);
    assert(4 == wid->tableau->slices_z[4][0]);
    assert(4 == wid->tableau->slices_z[5][0]);
    assert(4 == wid->tableau->slices_z[6][0]);
    assert(0 == wid->tableau->slices_z[7][0]);
    assert(0 == wid->tableau->slices_z[8][0]);

    // Testing the adjacency output
    struct adjacency_obj adj = widget_get_adjacencies(wid, 0);
    assert(0 == adj.n_adjacent);

    adj = widget_get_adjacencies(wid, 1);
    assert(0 == adj.n_adjacent);

    adj = widget_get_adjacencies(wid, 2);
    assert(4 == adj.n_adjacent);
   
    int32_t edges[4] = {3, 4, 5, 6}; 

    for (size_t i = 0; i < 4; i++)
    {
        bool found = 0;
        for (size_t j = 0; j < adj.n_adjacent; j++)
        {
            found |= (edges[i] == adj.adjacencies[j]);
        }
        assert(found);
    }

    for (size_t i = 3; i <= 6; i++)
    {
        adj = widget_get_adjacencies(wid, i);
        assert(1 == adj.n_adjacent);
        assert(2 == adj.adjacencies[0]);
    }

    widget_destroy(wid);
    return;
}

int main()
{
    test_toffoli();

    return 0;
}
