#ifndef PANDORA_H
#define PANDORA_H

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

struct pandora_map_t;
struct pandora_gate_t;

#include "instruction_table.h"
#include "consts.h"


struct pandora_map_t
{
    size_t n_qubits;
    size_t* map;
};

// Readout from pandora postgres object
struct pandora_gate_t {
    size_t id;
    size_t prev_q_1;
    size_t prev_q_2;
    size_t prev_q_3;
    char gate_name[20];
    size_t gate_param; 
    size_t _switch;
    size_t next_q_1;
    size_t next_q_2;
    size_t next_q_3;
    size_t label;
    size_t cl_ctrl;
}; 

#endif
