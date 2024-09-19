#include "postgres_result_casts.h"


/*
 * pg_result_print
 * Prints the output of a query
 * :: result : PGresult* :: Result object
 * Does not clear the connection after use
 */
void pg_result_print(PGresult* result)
{
    const size_t n_fields = PQnfields(result);
    const size_t n_tuples = PQntuples(result);

    for (size_t i = 0; i < n_fields; i++)
    {
        printf("%s\n", PQfname(result, i));
    }

    for (size_t i = 0; i < n_tuples; i++)
    {
        for (size_t j = 0; j < n_fields; j++)
        {
            printf("%s\n", PQgetvalue(result, i, j));
        }
    }
}


/*
 * pg_result_to_int32_t
 * Converts the first output of a PGresult object to an int32_t
 * :: result : PGresult* :: Result object
 * Does not clear the connection after use
 */
int32_t pg_result_to_int32_t(PGresult* result)
{
    const size_t n_tuples = PQntuples(result);
    assert(n_tuples > 0);

    char* pq_val = PQgetvalue(result, 0, 0);
    int32_t val;
    printf("Val: %s\n", pq_val);

    sscanf(pq_val, "%d", &val); 

    return val;
}


/*
 * pg_result_to_pandora_gates
 * Converts the output of a query on the pandora table to an array of pandora_gate_t structs  
 * :: result : PGresult* :: Result object
 * Does not clear the connection after use
 */
size_t pg_result_to_pandora_gates(PGresult* result, instruction_stream_u** stream)
{
    const size_t n_fields = PQnfields(result);
    const size_t n_tuples = PQntuples(result);

    // Allow virtual memory to handle this
    // Worst case is that every gate is a toffoli and must be decomposed 
    instruction_stream_u* dst = (instruction_stream_u*)malloc(sizeof(instruction_stream_u) * N_TOFFOLI_DECOMP * n_tuples);

    pandora_gate_t gate;
    size_t n_ops = 0;

    printf("N Tuples: %lu\n", n_tuples); 
   

    for (size_t i = 0; i < n_tuples; i++)
    {
        gate.gate_type = PQgetvalue(result, i, 0);
        gate.param = *(float*)PQgetvalue(result, i, 1);
        gate.qubit_0 = *(int*)PQgetvalue(result, i, 2);
        gate.qubit_1 = *(int*)PQgetvalue(result, i, 3);
        gate.qubit_2 = *(int*)PQgetvalue(result, i, 4);

        printf("Gate: %s\n", gate.gate_type); 

        n_ops += pandora_gate_parse(&gate, dst + n_ops);
    }

    *stream = dst;
    return n_ops;
}
