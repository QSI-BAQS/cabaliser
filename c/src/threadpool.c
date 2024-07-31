#define THREADPOOL_SRC 
#include "threadpool.h"

struct threadpool_t threadpool_create()
{
    threadpool_t tpool;
    tpool.alive = true;
    tpool.active_workers = 0;
    tpool.n_workers = N_WORKERS;

    tpool.job_queue = linked_list_create(); 

    pthread_mutex_init(&(tpool.queue_lock), NULL);
    for (size_t i = 0; i < N_WORKERS; i++)
    {
        pthread_create(tpool.workers + i, NULL, threadpool_worker, NULL);
    }

    return tpool;
}

void threadpool_destroy()
{
    pthread_mutex_lock(&(THREADPOOL_g.queue_lock)); 
    THREADPOOL_g.alive = false;
    for (size_t i = 0; i < N_WORKERS; i++)
    {
        pthread_join(THREADPOOL_g.workers[i], NULL); 
    } 
    linked_list_destroy(THREADPOOL_g.job_queue); 
    return;
}

/*
 * barrier_threadpool
 * Adds a task to the threadpool that is just a barrier
 * This forces threads to complete before the next task may resume 
 * :: 
 */
void barrier_threadpool()
{
//    pthread_barrier_t* bar = malloc(sizeof(pthread_barrier_t)); 
//    for (size_t i = 0; i < THREADPOOL_g.n_threads; i++)
//    {
//
//    }
    return;
}

void* barrier_fn(void* args)
{
    
    return NULL;
}


void distribute_task(void (*fn)(void*), void** args, size_t n_tasks)
{

}

/*
 * threadpool_worker
 * Threadpool worker function
 * :: args : void* ::
 * Polls the queue and pops items from it
 * Queue items should be function pointers and associated arguments  
 * Args are NULL
 * Returns NULL
 */
void* threadpool_worker(void* args)
{
//    const size_t thread_id = *((size_t*)args); 
    while (THREADPOOL_g.alive)
    {

    }    
    return NULL;
} 
