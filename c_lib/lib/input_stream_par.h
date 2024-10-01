#ifndef INPUT_STREAM_H
#define INPUT_STREAM_H

#include "threadpool.h"
#include "instructions.h"
#include "tableau_operations_par.h"
#include "widget.h"

/*
 * parse_instruction_block
 * Parses a block of instructions 
 * :: wid : widget_t* :: Current widget 
 * :: instructions : instruction_stream_u* :: Array of instructions 
 * :: n_instructions : const size_t :: Number of instructions in the stream 
 * This implementation may be replaced with a thread pool dispatcher
 */
void parse_instruction_block(
    widget_t* wid,
    instruction_stream_u* instructions,
    const size_t n_instructions);


/*
 * apply_local_cliffords
 * Empties the local clifford table and applies the local cliffords
 * :: wid : widget_t* :: The widget
 * Acts in place over the tableau and the clifford table
 */
void apply_local_cliffords_par(widget_t* wid);


#endif
