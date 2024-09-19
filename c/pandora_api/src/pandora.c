#include "pandora.h"


void pandora_init_db(char* db_name)
{
    pandora_t* pan = pandora_create(db_name);

    printf("Connection created\n");

    pandora_decorate_circuit(pan);

    printf("Decorated\n");

    pandora_destroy(pan);
}

/*
 *  pandora_to_instruction_stream
 *  Converts pandora table elements to instruction streams 
 *  :: stream : union instruction_stream_u* :: Array of stream objects to write to
 *  Returns the number of gates written to the stream object
 */
size_t pandora_to_instruction_stream(char* db_name, instruction_stream_u** stream, size_t layer)
{
    pandora_t* pan = pandora_create(db_name);
         
    return pandora_get_gates_layer(pan, layer, stream);
}


size_t pandora_get_n_qubits(char* db_name)
{
    pandora_t* pan = pandora_create(db_name);

    size_t n_qubits = pandora_connect_get_n_qubits(pan);

    pandora_destroy(pan);
    return n_qubits;
}

