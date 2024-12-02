#ifndef PANDORA_CONNECT_H
#define PANDORA_CONNECT_H

#include <stdint.h>
#include <string.h>

#include <arpa/inet.h>

#include "instruction_table.h"

#include "postgres_db.h"
#include "postgres_result_casts.h"

#define PANDORA_DB_STR "dbname = %s"
#define PANDORA_COUNT_N_QUBITS  "SELECT COUNT(*) FROM linked_circuit WHERE type = 'In'"

#define PANDORA_DECORATION "CALL decorate_circuit()" 

#define PANDORA_INITIAL  "SELECT type, qub_1 FROM linked_circuit WHERE type = 'In'"
#define PANDORA_GET_LAYER  "SELECT type, param, next_q1, next_q2, next_q3 FROM linked_circuit WHERE id = (SELECT decorated_circuit.id FROM decorated_circuit WHERE decorated_circuit.id = linked_circuit.id AND layer = $1)"


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

size_t pandora_connect_get_n_qubits(pandora_t* pan);


/*
 * pandora_decorate_circuit
 * Gets the number of qubits from the database
 * :: pan : pandora_t* :: Pandora connection object 
 * Returns the number of qubits as a size_t 
 */
void pandora_decorate_circuit(pandora_t* pan);

/*
 * pandora_get_gates_layer
 *
 * :: pan : pandora_t* :: Pandora connection object 
 * :: layer : size_t :: Layer to process 
 * :: stream : instruction_stream_u** :: Pointer to stream obj pointer  
 * Returns the number of gates in the stream 
 */
size_t pandora_get_gates_layer(pandora_t* pan, const size_t layer, instruction_stream_u** stream);

#endif

