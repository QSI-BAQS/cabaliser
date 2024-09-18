#ifndef POSTGRES_RESULT_CASTS_H
#define POSTGRES_RESULT_CASTS_H

#include <assert.h>

#include <stdio.h>
#include <stdlib.h>

#include <libpq-fe.h>

/*
 * pg_result_print
 * Prints the output of a query
 * :: result : PGresult* :: Result object
 * Does not clear the connection after use
 */
void pg_result_print(PGresult* result);

/*
 * pg_result_to_int32_t
 * Converts the first output of a PGresult object to an int32_t
 * :: result : PGresult* :: Result object
 * Does not clear the connection after use
 */
int32_t pg_result_to_int32_t(PGresult* result);


#endif
