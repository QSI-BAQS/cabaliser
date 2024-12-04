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
    pan->db_name = malloc(pan->db_name_len + 1);
    memcpy(pan->db_name, db_name, pan->db_name_len);
    pan->db_name[pan->db_name_len] = '\0';

    pan->conn = NULL;

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
void pandora_decorate_circuit(pandora_t* pan)
{
    pandora_connect(pan);

    // Conditionally decorate the table
    PGresult* result = PQexec(pan->conn, PANDORA_DECORATION);
    PQclear(result);

    return;
}

/*
 * pandora_disconnect
 * Disconnects the database connection in the pandora object
 * :: pan : pandora_t* :: Pandora connection object
 * Operates in place
 */
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
size_t pandora_connect_get_n_qubits(pandora_t* pan)
{
    pandora_connect(pan);

    PGresult* result = PQexec(pan->conn, PANDORA_COUNT_N_QUBITS);

    db_check_result_status(pan->conn, result);

    size_t n_qubits = pg_result_to_int32_t(result); 

    PQclear(result);

    return n_qubits;
}



// TODO tag
/*
 * pandora_get_gates_layer
 *
 * :: pan : pandora_t* :: Pandora connection object 
 * :: layer : size_t :: Layer to process 
 * :: stream : instruction_stream_u** :: Pointer to stream obj pointer  
 * Returns the number of gates in the stream 
 */
size_t pandora_get_gates_layer(pandora_t* pan, const size_t layer, instruction_stream_u** stream)
{ 
 
    uint32_t ht_layer = htonl(layer);
    char* param_values[1] = {(char*)&ht_layer}; 

    const static int n_params = 1;
    const static Oid *param_types = NULL;
    const static int param_lengths[1] = {sizeof(int)}; 
    const static int param_formats[1] = {POSTGRES_BINARY_FORMAT};

    pandora_connect(pan);
    PGresult* result = PQexecParams(
        pan->conn, 
        PANDORA_GET_LAYER,
        n_params, 
        param_types,
        (const char* const*)param_values,
        param_lengths,
        param_formats,
        POSTGRES_BINARY_FORMAT 
    );
    db_check_result_status(pan->conn, result);
    
    size_t n_gates = pg_result_to_pandora_gates(result, stream);

    PQclear(result);
    return n_gates;
}
