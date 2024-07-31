#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <pthread.h>
#include <stdbool.h>

#include "linked_list.h"

#define N_WORKERS 8

struct threadpool_t {
    size_t n_workers;
    size_t active_workers;
    pthread_t workers[N_WORKERS]; 
    struct linked_list_t* job_queue; 
    pthread_mutex_t queue_lock;  
    bool alive; // Set to false to kill workers 
};
typedef struct threadpool_t threadpool_t;

#ifdef THREADPOOL_SRC
    threadpool_t THREADPOOL_g;
#else
    extern threadpool_t THREADPOOL_g;
#endif 


struct threadpool_arg
{
    size_t thread_id;
    void* args;
}; 

struct threadpool_barrier_t
{
    pthread_barrier_t* barrier;         
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



#endif
