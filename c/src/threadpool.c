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
 * barrier_fn
 * Function to barrier workers 
 * :: args : struct threadpool_barrier_t* :: Barrier object, declared as void* 
 * Barriers all threads, on release uses the final thread to free the barrier object created in  
 * threadpool_barrier
 * This requires a semaphore library and may not function on MacOS machines
 */
void* barrier_fn(void* args)
{
    // Barrier on threads
    pthread_barrier_wait(((pthread_barrier_t*)args));
     
    // Semaphore set to n_threads - 1, such that the final thread will throw EAGAIN
    int err = sem_trywait(&(((threadpool_barrier_t*)args)->sem));

    // Operation is threadsafe as all threads must have passed the previous line
    // As a result even if an interrupt occurs at this point only one thread will trigger the free call
    
    // Last thread has exited barrier
    if (EAGAIN == err)
    {
       free(args); 
    }

    return NULL;
}

/*
 * barrier_threadpool
 * Adds a task to the threadpool that is just a barrier
 * This forces threads to complete before the next task may resume 
 * :: 
 */
void barrier_threadpool()
{
    struct threadpool_barrier_t* bar = malloc(sizeof(struct threadpool_barrier_t)); 
     
    pthread_barrier_init(&(bar->barrier), NULL, THREADPOOL_g.nthreads); 
    sem_init(&(bar->sem), 0, THREADPOOL_g.nthreads - 1);  // Last thread to exit will trigger EAGAIN

    for (size_t i = 0; i < THREADPOOL_g.n_threads; i++)
    {
        add_task(barrier_fn, bar, sizeof(pthread_barrier_t));
    }
    return;
}





/*
 * Adds a job to the threadpool
 */
void add_task(void (*fn)(void*), void* args, const size_t n_bytes_args)
{
    struct threadpool_job job;
    job.fn = fn;
    job.args = args; 
    const size_t n_bytes = sizeof(fn) + n_bytes_args;

    pthread_mutex_lock(&THREADPOOL_g.queue_lock); 
    linked_list_push(THREADPOOL_g.job_queue, &job, n_bytes);
    pthread_mutex_unlock(&THREADPOOL_g.queue_lock); 
    return;
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

        pthread_mutex_lock(&(THREADPOOL_g.queue_lock));
        struct threadpool_job* job = linked_list_pop(THREADPOOL_g.job_queue); 
        pthread_mutex_unlock(&(THREADPOOL_g.queue_lock));

        if (NULL != job)
        {
            job->fn(job->args);   
            free(job->args); 
            free(job);
        }
    }    
    return NULL;
} 
