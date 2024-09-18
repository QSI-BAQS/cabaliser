#ifndef PANDORA_CONNECT_H
#define PANDORA_CONNECT_H

#include <stdint.h>
#include <string.h>

#include "postgres_db.h"
#include "postgres_result_casts.h"

#define PANDORA_DB_STR "dbname = %s"
#define PANDORA_COUNT_N_QUBITS  "SELECT COUNT(*) FROM linked_circuit_qubit WHERE type = 'In'"
#define PANDORA_INITIAL  "SELECT type, qub_1, next_q1 FROM linked_circuit_qubit WHERE type = 'In'"

#define PANDORA_DECORATED_TABLE "CREATE TABLE IF NOT EXISTS decorated_circuit(id INT PRIMARY KEY, gate INT REFERENCES linked_circuit_qubit, layer INT)"

struct pandora_t {
    size_t db_name_len;
    char* db_name;
    size_t tag_name_len;
    char* tag_name;
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

