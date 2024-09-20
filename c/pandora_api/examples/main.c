#include "pandora_connect.h"

int main()
{
    char* db_name = "postgres";

    size_t n_qubits = pandora_get_n_qubits(db_name);
 
    widget_t* wid = widget_create(n_qubits, 3 * n_qubits);

    pandora_t* pan = pandora_create(db_name);

//    printf("%lu\n", pandora_get_n_qubits(db_name));

    //pandora_init_db(db_name);
    instruction_stream_u* stream = NULL;

    size_t n_gates = 1;
    for (size_t layer = 1; n_gates > 0; layer++)
    {
        printf("Layer: %lu\n", layer);
        n_gates = pandora_get_gates_layer(pan, layer, &stream);

        parse_instruction_block(wid, stream, n_gates);  

        printf("Gates at layer %lu : %lu\n", layer, n_gates );
        free(stream);
    }

    pandora_destroy(pan); 
   
    widget_decompose(wid);

    widget_destroy(wid);   
 
    return 0;

}
