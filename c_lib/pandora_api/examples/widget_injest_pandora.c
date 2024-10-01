#include "pandora.h"

int main()
{
    char* db_name = "postgres";

    size_t n_qubits = pandora_get_n_qubits(db_name);
    //printf("N qubits: %lu\n", n_qubits);

    widget_t* wid = widget_create(1, 10);

    pandora_load_db(wid, db_name);   

    return 0;
}
