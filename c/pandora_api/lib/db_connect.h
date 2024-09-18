#ifndef PANDORA_CONNECT_H
#define PANDORA_CONNECT_H

#include <stdio.h>
#include <stdlib.h>

#include <libpq-fe.h>


/*
 * db_connect
 * Creates a connection object;
 * :: conninfo : char* :: String handling connection data
 */
PGconn* db_connect(const char* conninfo);

/*
 * db_conn_exit
 * Disconnects from the database and terminates
 * :: conn : PGconn* :: Connection object  
 */
void db_conn_exit(PGconn *conn);


/*
 * db_conn_teardown
 * Finishes a connection 
 * :: conn : PGconn* :: Connection object  
 */
void db_conn_teardown(PGconn *conn);




#endif
