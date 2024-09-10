#include "pandora.h"

struct pandora_map_t* pandora_qubit_map_create(const size_t n_qubits)
{
    struct pandora_map_t* map = NULL;
    const int err_code = posix_memalign((void**) &(map->map), (size_t)CACHE_SIZE, sizeof(size_t) * n_qubits);
    assert(0 == err_code);
    return NULL;
}

/*
 *  pandora_to_instruction_stream
 *  Converts pandora table elements to instruction streams 
 *  :: map : struct pandora_map* :: Map tracking qubits
 *  :: gate : struct pandora_gate_t* gate :: Pandora gate object 
 *  :: stream : union instruction_stream_u* :: Array of stream objects to write to
 *  Returns the number of gates written to the stream object
 */
size_t pandora_to_instruction_stream(
    struct pandora_map_t* map, 
    struct pandora_gate_t* gate, 
    instruction_stream_u* stream)
{

    return 0;
}
