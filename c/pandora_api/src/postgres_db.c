#include "postgres_db.h"

/*
 * db_connect
 * Creates a connection object;
 * :: conninfo : char* :: String handling connection data
 */
PGconn* db_connect(const char* conninfo)
{

    PGconn* conn = PQconnectdb(conninfo);
    db_connection_status(conn);
    db_set_secure_search(conn);

    return conn;
} 


/*
 * db_conn_exit
 * Disconnects from the database and terminates
 * :: conn : PGconn* :: Connection object  
 */
void db_conn_exit(PGconn *conn)
{
    db_conn_teardown(conn);
    exit(1);
}


/*
 * db_conn_teardown
 * Finishes a connection 
 * :: conn : PGconn* :: Connection object  
 */
void db_conn_teardown(PGconn *conn)
{
    PQfinish(conn);
}


/*
 * db_connection_status
 * Tests the status of a connection
 * :: conn : PGconn* :: Active DB connection 
 */
void db_connection_status(PGconn* conn)
{
    if (CONNECTION_OK != PQstatus(conn))
    {
        fprintf(
            stderr, 
            "Database connection failed %s\n",
             PQerrorMessage(conn)
        ); 
    } 
}

/*
 * db_set_secure_search
 * Best practice function
 * :: conn : PGconn* :: Connection object
 */
void db_set_secure_search(PGconn *conn)
{
    PGresult* res = PQexec(conn, DB_SET_SECURE_SEARCH);

    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        fprintf(stderr, "SET failed: %s", PQerrorMessage(conn));
        PQclear(res);
        db_conn_exit(conn);
    }
    PQclear(res);
}
