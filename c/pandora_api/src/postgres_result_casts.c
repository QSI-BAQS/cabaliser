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
