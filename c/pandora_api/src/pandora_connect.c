#include "pandora_connect.h"

/*
 * pandora_connect
 * Creates a connection object to the pandora database
 * Thin wrapper around db_connect
 */
PGconn* pandora_connect()
{
    static const char* conn_info = PANDORA_DB;  
    return db_connect(conn_info); 
}
