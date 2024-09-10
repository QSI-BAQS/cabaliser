#define PANDORA_GATE_SRC 
#include "pandora_gates.h"

size_t pandora_Pauli(struct pandora_map_t* map, struct pandora_gate_t* gate,  instruction_stream_u* stream)
{

    stream[0].single.opcode = GATE_TO_PAULI_INSTRUCTION(gate);
    //size_t arg = gate->prev_q_1;  
    return 0;
}

size_t pandora_AND(struct pandora_gate_t* gate, struct pandora_map_t* map, instruction_stream_u* stream)
{
    // Discuss how to resolve AND gadget
    return 0;
}

 // 'And'
size_t pandora_C(struct pandora_gate_t* gate, struct pandora_map_t* map, instruction_stream_u* stream)
{
    return 0;
}

 // CNOT, CZ
size_t pandora_global_phase(struct pandora_gate_t* gate, struct pandora_map_t* map, instruction_stream_u* stream)
{
    return 0;
}


size_t pandora_HPow(struct pandora_gate_t* gate, struct pandora_map_t* map, instruction_stream_u* stream)
{
    return 0;
}

// Trivial operation, should be used to indicate an allocation
size_t pandora_In(struct pandora_gate_t* gate, struct pandora_map_t* map, instruction_stream_u* stream)
{
    return 0;
}

// Terminal operation
size_t pandora_Measure(struct pandora_gate_t* gate, struct pandora_map_t* map, instruction_stream_u* stream)
{
    return 0;
}

size_t pandora_Out(struct pandora_gate_t* gate, struct pandora_map_t* map, instruction_stream_u* stream)
{
    return 0;
}

/*
 * pandora_R
 * Distinguish between Rx, Ry, Rz
 * Special cases exist for rotation angles that resolve to individual Paulis
 * :: map : struct pandora_map_t* :: Qubit map
 * :: map : struct pandora_map_t* :: Qubit map
 * :: map : struct pandora_map_t* :: Qubit map
 */
size_t pandora_R(struct pandora_gate_t* gate, struct pandora_map_t* map, instruction_stream_u* stream)
{
    switch (gate->gate_name[2]) //TODO decide on switch char
    {
        case 'x':
            stream[0].single.opcode = _H_;
            stream[0].single.arg = 0; // TODO arg tracking 

            stream[0].rz.opcode = _RZ_;
            stream[1].rz.arg = 0; // TODO arg tracking 
            stream[1].rz.tag = 0; // TODO tagging;
            stream[2].single.opcode = _H_;
            stream[2].single.arg = 0; // TODO arg tracking         
            return 3;
        case 'y':
            stream[0].single.opcode = _S_;
            stream[0].single.arg = 0; // TODO arg tracking 

            stream[1].single.opcode = _H_;
            stream[1].single.arg = 0; // TODO arg tracking 
            stream[2].rz.opcode = _RZ_;

            stream[2].rz.arg = 0; // TODO arg tracking 
            stream[2].rz.tag = 0; // TODO tagging;

            stream[3].single.opcode = _H_;
            stream[3].single.arg = 0; // TODO arg tracking         
            stream[4].single.opcode = _S_;
            stream[4].single.arg = 0; // TODO arg tracking 

            return 5;
    
        case 'z': // Rz is native
            stream[0].rz.opcode = _RZ_;
            stream[0].rz.arg = _RZ_;
            stream[0].rz.tag = 0 // TODO tagging;
            return 1;
    }
    
}

size_t pandora_Toffoli(struct pandora_gate_t* gate, struct pandora_map_t* map, instruction_stream_u* stream)
{
    // Decomp
    return 0;
}


size_t pandora_XPow(struct pandora_gate_t* gate, struct pandora_map_t* map, instruction_stream_u* stream)
{
    return 0;
}


size_t pandora_YPow(struct pandora_gate_t* gate, struct pandora_map_t* map, instruction_stream_u* stream)
{
    return 0;
}


size_t pandora_ZPow(struct pandora_gate_t* gate, struct pandora_map_t* map, instruction_stream_u* stream)
{
    return 0;
}
