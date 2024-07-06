#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include <stdint.h>
#include <string.h>
#include <stdlib.h>


#include "consts.h"

#define MAX_INSTRUCTION_SEQUENCE_LEN (8)
#define CLIFFORD_OPCODE_WIDTH (1)

#define I    0b0000 
#define Id   0b0001 
#define H    0b0010
#define Hd   0b0011
#define P    0b0110 
#define Pd   0b0111

#define X    0b1000 
#define Z    0b0100
#define Y    0b1100

#define CNOT 0b010000 
#define CZ   0b100000

/*
 * instruction_struct
 * Simple struct that allows for easy instruction parsing  
 */
struct instruction_t
{
    uint8_t opcode;
    uint32_t arg_a;
    uint32_t arg_b;
};
typedef struct instruction_t instruction_t;

/*
 * instruction_queue
 * Implemented a fixed length queue of single qubit Clifford instructions
 * Single qubit additions to the queue simply twirl the queue 
 * Two qubit additions to the queue either:
 *   - Commute through and twirl performing a local operation on the graph state 
 *   - Trigger a dump of the instructions to the tableau 
 */
struct instruction_queue
{
    size_t n_qubits;
    instruction_t** table;
};
typedef struct instruction_queue instruction_queue_t;


#endif
