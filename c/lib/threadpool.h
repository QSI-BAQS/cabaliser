#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>

#include "tableau.h" 
#include "linked_list.h"

#define N_WORKERS (2)
#define NULL_TARG (~(0ull))  // Null target

struct threadpool_t {
    size_t n_workers;
    size_t active_workers;
    pthread_t workers[N_WORKERS]; 
    struct linked_list_t* job_queue; 
    pthread_mutex_t queue_lock;  
    bool alive; // Set to false to kill workers 
    __uint128_t dependent_qubits; // Tracks qubits under operation
};
typedef struct threadpool_t threadpool_t;

#ifdef THREADPOOL_SRC
    threadpool_t THREADPOOL_g;
    bool THREADPOOL_INITIALISED = 0;
#else
    extern threadpool_t THREADPOOL_g;
#endif 


struct threadpool_job
{
    void (*fn)(void*);
    void* args;
};

struct threadpool_arg
{
    size_t thread_id;
    void* job;
}; 

struct threadpool_barrier_t
{
    pthread_barrier_t barrier;         
    uint32_t thread_count;
};

struct distributed_tableau_op
{
    tableau_t* tab;
    size_t ctrl;
    size_t targ; 
    size_t start;
    size_t stop;
};

/*
 * threadpool_worker
 * Threadpool worker function
 * :: args : void* ::
 * Polls the queue and pops items from it
 * Queue items should be function pointers and associated arguments  
 * Args are NULL
 * Returns NULL
 */
void* threadpool_worker(void* args);

/*
 * Adds a job to the threadpool
 */
void threadpool_add_task(void (*fn)(void*), void* args);

/*
 * threadpool_distribute_tableau_operation_single
 * Distributes a tableau operation over the workers 
 * :: tab : tableau_t* :: Tableau to operate over
 * :: fn : void* (*)(void*) :: Function to distriute
 * :: ctrl : const size_t :: First qubit 
 * :: targ : const size_t :: Second qubit, set to NULL_TARG to null
 */
void threadpool_distribute_tableau_operation(
    tableau_t* tab,
    void (*fn)(void*),
    const size_t ctrl,
    const size_t targ);

/*
 * threadpool_barrier 
 * Adds a task to the threadpool that is just a barrier
 * This forces threads to complete before the next task may resume 
 * :: 
 */
void threadpool_barrier();

/*
 * threadpool_join
 */
void threadpool_join();

/*
 * threadpool_join
 */
void threadpool_destroy();

/*
 * threadpool_init
 */
void threadpool_init();


#endif
