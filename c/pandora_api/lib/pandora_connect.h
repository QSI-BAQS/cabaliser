#ifndef PANDORA_CONNECT_H
#define PANDORA_CONNECT_H

#include <stdint.h>
#include <string.h>

#include "postgres_db.h"
#include "postgres_result_casts.h"

#define PANDORA_DB_STR "dbname = %s"
#define PANDORA_COUNT_N_QUBITS  "SELECT COUNT(*) FROM linked_circuit_qubit WHERE type = 'In'"

struct pandora_t {
    size_t db_name_len;
    char* db_name;
    PGconn* conn; 
};
typedef struct pandora_t pandora_t; 


/*
 * pandora_create
 * constructor for Pandora connections
 * :: db_name : char* :: Name of the target database
 */
pandora_t* pandora_create(char* db_name);
void pandora_destroy(pandora_t* pan);

/*
 * pandora_connect
 * Creates a connection object to the pandora database
 * Thin wrapper around db_connect
 */
void pandora_connect(pandora_t* pan);
void pandora_disconnect(pandora_t* pan);

size_t pandora_get_n_qubits(pandora_t* pan);


#endif

