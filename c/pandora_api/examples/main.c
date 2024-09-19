#include "pandora_connect.h"

int main()
{
    char* db_name = "postgres";

    pandora_t* pan = pandora_create(db_name);

//    printf("%lu\n", pandora_get_n_qubits(db_name));

    //pandora_init_db(db_name);
    instruction_stream_u* stream = NULL;

    size_t n_gates = 1;
    for (size_t layer = 1; n_gates > 0; layer++)
    {
        printf("Layer: %lu\n", layer);
        n_gates = pandora_get_gates_layer(pan, layer, &stream);

        printf("Gates at layer %lu : %lu\n", layer, n_gates );
        free(stream);
    }
   
    pandora_destroy(pan); 
      
    return 0;

}
