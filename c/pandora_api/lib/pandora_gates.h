#ifndef PANDORA_GATES_H
#define PANDORA_GATES_H

#include <stdint.h>
#include <stddef.h>

#include "instruction_table.h"
#include "instructions.h"


#include "pandora.h"

#define N_TOFFOLI_DECOMP (14) 

typedef uint8_t pandora_instruction_t;

/*
 * Pandora gate return object
 * Small wrapper for pandora gate objects
 */
struct pandora_gate_t {
    char* gate_type;
    float param;
    uint32_t qubit_0;
    uint32_t qubit_1;
    uint32_t qubit_2;
};

typedef struct pandora_gate_t pandora_gate_t;


size_t pandora_gate_parse(pandora_gate_t* gate, instruction_stream_u* stream);


size_t pandora_Pauli(struct pandora_gate_t* gate,  instruction_stream_u* stream);

size_t pandora_AND(struct pandora_gate_t*, instruction_stream_u* stream);

 // 'And'
size_t pandora_C(struct pandora_gate_t*, instruction_stream_u* stream);

 // CNOT, CZ
size_t pandora_global_phase(struct pandora_gate_t*, instruction_stream_u* stream);


size_t pandora_HPow(struct pandora_gate_t*, instruction_stream_u* stream);


size_t pandora_In(struct pandora_gate_t*, instruction_stream_u* stream);

 // In
size_t pandora_Measure(struct pandora_gate_t*, instruction_stream_u* stream);

 // Measure
size_t pandora_Out(struct pandora_gate_t*, instruction_stream_u* stream);

 // Out
size_t pandora_R(struct pandora_gate_t*, instruction_stream_u* stream);

 // R{x, y, z} 
size_t pandora_Toffoli(struct pandora_gate_t*, instruction_stream_u* stream);

 // Toffoli
size_t pandora_XPow(struct pandora_gate_t*, instruction_stream_u* stream);


size_t pandora_YPow(struct pandora_gate_t*, instruction_stream_u* stream);


size_t pandora_ZPow(struct pandora_gate_t*, instruction_stream_u* stream);



#ifdef PANDORA_GATE_SRC 

    const static instruction_t pauli_map_g[3] = {_X_, _Y_, _Z_}; 
    #define GATE_TO_PAULI_INSTRUCTION(pandora_gate) (pauli_map_g[gate->gate_name[6] - 'X'](pandora_gate)) // NB: Using strings to do this is silly 


// Lookup table for translating pandora gates to an instruction stream
// Table is conditioned on the first character of the "gate type" field.
size_t (*const PANDORA_GATE_TRANSLATION[32])(struct pandora_gate_t*, instruction_stream_u*) = {
    pandora_AND, // 'And'
    NULL,
    pandora_C, // CNOT, CZ
    NULL,
    NULL,
    NULL,
    pandora_global_phase,
    pandora_HPow,
    pandora_In, // In
    NULL,
    NULL,
    NULL,
    pandora_Measure, // Measure
    NULL,
    pandora_Out, // Out
    NULL,
    NULL,
    pandora_R, // R{x, y, z} 
    NULL,
    pandora_Toffoli, // Toffoli
    NULL,
    NULL,
    NULL,
    pandora_XPow,
    pandora_YPow,
    pandora_ZPow,
    NULL,
    NULL,
    NULL,
    NULL,
    pandora_Pauli, // _Pauli{X, Y, Z} 
}; 

#define PANDORA_PAULI_STR_OFFSET 7
#define PANDORA_PAULI_OFFSET 'X' 


#else 
    extern const size_t (*PANDORA_GATE_TRANSLATION[32])(struct pandora_gate_t*, instruction_stream_u*); 
#endif

#endif
