#include "pandora.h"


/*
 *  pandora_to_instruction_stream
 *  Converts pandora table elements to instruction streams 
 *  :: map : struct pandora_map* :: Map tracking qubits
 *  :: gate : struct pandora_gate_t* gate :: Pandora gate object 
 *  :: stream : union instruction_stream_u* :: Array of stream objects to write to
 *  Returns the number of gates written to the stream object
 */
size_t pandora_to_instruction_stream(
    instruction_stream_u* stream)
{

    return 0;
}
