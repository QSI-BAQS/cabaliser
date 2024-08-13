#define PANDORA_GATE_SRC 
#include "pandora_gates.h"


size_t pandora_Pauli(struct pandora_map_t* map, struct pandora_gate_t* gate,  instruction_stream_u* stream)
{

    stream[0].single.opcode = GATE_TO_PAULI_INSTRUCTION(gate);
    //size_t arg = gate->prev_q_1;  
    return 0;
}

size_t pandora_AND(struct pandora_map_t*, struct pandora_gate_t*, instruction_stream_u* stream)
{
    return 0;
}

 // 'And'
size_t pandora_C(struct pandora_map_t*, struct pandora_gate_t*, instruction_stream_u* stream)
{
    return 0;
}

 // CNOT, CZ
size_t pandora_global_phase(struct pandora_map_t*, struct pandora_gate_t*, instruction_stream_u* stream)
{
    return 0;
}


size_t pandora_HPow(struct pandora_map_t*, struct pandora_gate_t*, instruction_stream_u* stream)
{
    return 0;
}


size_t pandora_In(struct pandora_map_t*, struct pandora_gate_t*, instruction_stream_u* stream)
{
    return 0;
}

 // In
size_t pandora_Measure(struct pandora_map_t*, struct pandora_gate_t*, instruction_stream_u* stream)
{
    return 0;
}

 // Measure
size_t pandora_Out(struct pandora_map_t*, struct pandora_gate_t*, instruction_stream_u* stream)
{
    return 0;
}

 // Out
size_t pandora_R(struct pandora_map_t*, struct pandora_gate_t*, instruction_stream_u* stream)
{
    return 0;
}

 // R{x, y, z} 
size_t pandora_Toffoli(struct pandora_map_t*, struct pandora_gate_t*, instruction_stream_u* stream)
{
    return 0;
}

 // Toffoli
size_t pandora_XPow(struct pandora_map_t*, struct pandora_gate_t*, instruction_stream_u* stream)
{
    return 0;
}


size_t pandora_YPow(struct pandora_map_t*, struct pandora_gate_t*, instruction_stream_u* stream)
{
    return 0;
}


size_t pandora_ZPow(struct pandora_map_t*, struct pandora_gate_t*, instruction_stream_u* stream)
{
    return 0;
}

