#ifndef PANDORA_H
#define PANDORA_H

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

#include "instruction_table.h"
#include "consts.h"

#include "pandora_connect.h"
#include "pandora_gates.h"


void pandora_init_db(char* db_name);

/*
 *  pandora_to_instruction_stream
 *  Converts pandora table elements to instruction streams 
 *  :: stream : union instruction_stream_u* :: Array of stream objects to write to
 *  Returns the number of gates written to the stream object
 */
size_t pandora_to_instruction_stream(char* db_name, instruction_stream_u** stream, size_t layer);

size_t pandora_get_n_qubits(char* db_name);


#endif
