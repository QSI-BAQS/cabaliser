#include "pandora_connect.h"

/*
 * pandora_create
 * constructor for Pandora connections
 * :: db_name : char* :: Name of the target database
 * Returns a heap allocated pandora connection struct
 * The connection is initially active
 */
pandora_t* pandora_create(char* db_name)
{
    pandora_t* pan = malloc(sizeof(pandora_t));

    pan->tag_name = NULL;

    pan->db_name_len =  strlen(db_name);
    pan->db_name = malloc(pan->db_name_len);
    memcpy(pan->db_name, db_name, pan->db_name_len);

    pandora_connect(pan);
    return pan;
}

/*
 * pandora_create_tagged
 * constructor for Pandora connections
 * :: db_name : char* :: Name of the target database
 * :: tag : char* :: Name of the target database
 * Returns a heap allocated pandora connection struct
 * The connection is initially active
 */
pandora_t* pandora_create_tagged(char* db_name, char* tag)
{
    pandora_t* pan = pandora_create(db_name);

    pan->tag_name_len = strlen(tag);
    pan->tag_name = malloc(pan->tag_name_len);
    memcpy(pan->tag_name, tag, pan->tag_name_len);

    return pan;
}



/*
 * pandora_decorate_circuit
 * Gets the number of qubits from the database
 * :: pan : pandora_t* :: Pandora connection object 
 * Returns the number of qubits as a size_t 
 */
void pandora_decorate_circuit_tagged(pandora_t* pan);
void pandora_decorate_circuit_untagged(pandora_t* pan);
void pandora_decorate_circuit(pandora_t* pan)
{
    pandora_connect(pan);

    // Conditionally decorate the table
    PGresult* result = PQexec(pan->conn, PANDORA_DECORATED_TABLE);
    db_check_result_status(pan->conn, result);
    PQclear(result);

    // Decorate the circuit 
    if (NULL == pan->tag_name)
    {
        pandora_decorate_circuit_untagged(pan);
    }
    else
    {
        pandora_decorate_circuit_tagged(pan);
    }

    return;
}

void pandora_decorate_circuit_untagged(pandora_t* pan)
{

    // Conditionally decorate the table
    PGresult* result = PQexec(pan->conn, PANDORA_DECORATED_TABLE);
    db_check_result_status(pan->conn, result);
    PQclear(result);

    return;
}
void pandora_decorate_circuit_tagged(pandora_t* pan)
{

    // Conditionally decorate the table
    PGresult* result = PQexec(pan->conn, PANDORA_DECORATED_TABLE);
    db_check_result_status(pan->conn, result);
    PQclear(result);

    return;
}


void pandora_disconnect(pandora_t* pan)
{
    if (NULL != pan->conn)
    {
        db_conn_teardown(pan->conn);
    }
    pan->conn = NULL; 
}


/*
 * pandora_destroy
 * Destructor method for pandora connection struct
 * Disconnects the connection and frees the associated memory
 */
void pandora_destroy(pandora_t* pan)
{
    pandora_disconnect(pan);

    if (NULL != pan->tag_name)
    {
        free(pan->tag_name);
    }

    free(pan->db_name);
    free(pan);
}

/*
 * pandora_connect
 * Creates a connection object to the pandora database
 * :: pan : pandora_t* :: Pandora connection object
 * Acts in place 
 */
void pandora_connect(pandora_t* pan)
{
    if (NULL != pan->conn)
    { 
        return;
    }
    const size_t db_name_len = strlen(pan->db_name);  
    const size_t db_str_len = strlen(PANDORA_DB_STR) + db_name_len; // Will be 3 bytes too big, we will live
    
    char* conn_info = (char*)malloc(db_str_len); 
    sprintf(conn_info, PANDORA_DB_STR, pan->db_name);
    PGconn* conn = db_connect(conn_info); 
    free(conn_info);

    pan->conn = conn;
    return; 
}


/*
 * pandora_get_n_qubits
 * Gets the number of qubits from the database
 * :: pan : pandora_t* :: Pandora connection object 
 * Returns the number of qubits as a size_t 
 */
size_t pandora_get_n_qubits(pandora_t* pan)
{
    pandora_connect(pan);

    PGresult* result = PQexec(pan->conn, PANDORA_COUNT_N_QUBITS);

    db_check_result_status(pan->conn, result);

    size_t n_qubits = pg_result_to_int32_t(result); 

    PQclear(result);

    return n_qubits;
}


/*
 * pandora_reset_visited
 *
 *
 */


// TODO tag
/*
 * pandora_get_initial_gates
 *
 * :: pan : pandora_t* :: Pandora connection object 
 *
 * Returns a gate stream object
 */
void* pandora_get_initial_gates(pandora_t* pan)
{ 
    pandora_connect(pan);

    PGresult* result = PQexec(pan->conn, PANDORA_COUNT_N_QUBITS);

    db_check_result_status(pan->conn, result);

//    void* initial_gate_arr = pg_result_to_gate_stream(result);
//    void* initial_gate_arr = pg_result_to_gate_stream(result);


    PQclear(result);
    return NULL;
}
