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

    pan->db_name_len =  strlen(db_name);
    pan->db_name = malloc(pan->db_name_len);
    memcpy(pan->db_name, db_name, pan->db_name_len);

    pandora_connect(pan);
    return pan;
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
