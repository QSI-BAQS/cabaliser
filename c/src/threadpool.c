#define THREADPOOL_SRC 
#include "threadpool.h"

/*
 * threadpool_create
 * Constructor for the threadpool object
 */
void threadpool_init()
{
    if (!THREADPOOL_INITIALISED)
    { 
        THREADPOOL_g.alive = true;
        THREADPOOL_g.active_workers = 0;
        THREADPOOL_g.n_workers = N_WORKERS;

        THREADPOOL_g.job_queue = linked_list_create(); 

        pthread_mutex_init(&(THREADPOOL_g.queue_lock), NULL);
        for (size_t i = 0; i < N_WORKERS; i++)
        {
            pthread_create(THREADPOOL_g.workers + i, NULL, threadpool_worker, NULL);
        }
        THREADPOOL_INITIALISED = 1;

        printf("Threadpool created!\n");
    } 
    return;
}

/*
 * threadpool_destroy
 * Destructor for the threadpool object
 */
void threadpool_destroy()
{
    threadpool_join();
    pthread_mutex_lock(&(THREADPOOL_g.queue_lock)); 
    THREADPOOL_g.alive = false;
    for (size_t i = 0; i < N_WORKERS; i++)
    {
        pthread_join(THREADPOOL_g.workers[i], NULL); 
    } 
    linked_list_destroy(THREADPOOL_g.job_queue); 
    pthread_mutex_unlock(&(THREADPOOL_g.queue_lock)); 

    return;
}


/*
 * __dependency_check
 * Performs a dependency check on the current arg
 * :: arg : const size_t :: Argument to check for dependencies 
 * Checks if the arg is a factor of the current dependency
 * If so, then triggeres a barrier, otherwise
 */
static inline
void __dependency_check(const size_t arg)
{
    // Non-target qubit
    if (NULL_TARG == arg)
    {
        return;
    } 
    // Either shares a common factor or triggers an overflow
    // Either way, barrier than reset
    if (
        (0 == (THREADPOOL_g.dependent_qubits % (arg + 2)))
        || ((THREADPOOL_g.dependent_qubits * arg) < THREADPOOL_g.dependent_qubits))
    { 
      threadpool_barrier();   
      THREADPOOL_g.dependent_qubits = (arg + 2);
    }
    else
    {
        THREADPOOL_g.dependent_qubits *= (arg + 2);
    }
    return;
}

/*
 * __barrier_fn
 * Function to barrier workers 
 * :: args : struct threadpool_barrier_t* :: Barrier object, declared as void* 
 * Barriers all threads, on release uses the final thread to free the barrier object created in  
 * threadpool_barrier
 * This requires a semaphore library and may not function on MacOS machines
 */
void __barrier_fn(struct threadpool_barrier_t** args)
{
    printf("Waiting on barrier\n");
    struct threadpool_barrier_t* bar = *args;
    // Barrier on threads
    pthread_barrier_wait(&(bar->barrier));
     
    // Semaphore set to n_threads - 1, such that the final thread will throw EAGAIN
    //  REPLACE with atomic int 
    //  int err = sem_trywait(&(bar->sem));

    // Operation is threadsafe as all threads must have passed the previous line
    // As a result even if an interrupt occurs at this point only one thread will trigger the free call
    
    // Last thread has exited barrier
}

/*
 * threadpool_barrier 
 * Adds a task to the threadpool that is just a barrier
 * This forces threads to complete before the next task may resume 
 * :: 
 */
void threadpool_barrier()
{
    printf("Creating Barrier\n");

    // Something for the job free-er to free
    struct threadpool_barrier_t* bar = malloc(sizeof(struct threadpool_barrier_t)); 

    pthread_barrier_init(&(bar->barrier), NULL, THREADPOOL_g.n_workers); 
    //sem_init(&(bar->sem), 0, THREADPOOL_g.n_workers - 1);  // Last thread to exit will trigger EAGAIN

    for (size_t i = 0; i < THREADPOOL_g.n_workers; i++)
    {
        struct threadpool_barrier_t** bar_ptr = malloc(sizeof(struct threadpool_barrier_t*)); 
        bar_ptr = &bar; 
        threadpool_add_task((void (*)(void*))__barrier_fn, bar_ptr);
    }
    return;
}


/*
 * threadpool_join 
 * Block execution of main thread until all workers in threadpool have passed this point
 */
void threadpool_join()
{
    struct threadpool_barrier_t* bar = malloc(sizeof(struct threadpool_barrier_t)); 
    pthread_barrier_init(&(bar->barrier), NULL, THREADPOOL_g.n_workers + 1); 
    //sem_init(&(bar->sem), 0, THREADPOOL_g.n_workers + 1);  // Last thread to exit will trigger EAGAIN

    for (size_t i = 0; i < THREADPOOL_g.n_workers; i++)
    {
        threadpool_add_task((void (*)(void*))__barrier_fn, bar);
    }
    __barrier_fn(&bar);
    return;
}

/*
 * threadpool_add_task
 * Adds a job to the threadpool
 * :: fn : void (*)(void*) :: Worker function
 * :: args : void* :: Args for the worker
 */
void threadpool_add_task(void (*fn)(void*), void* args)
{
    struct threadpool_job job;
    job.fn = (void*)fn;
    job.args = args; 
    const size_t n_bytes = sizeof(struct threadpool_job);

    printf("Acqiring List Mutex\n");
    pthread_mutex_lock(&THREADPOOL_g.queue_lock); 
    linked_list_push(THREADPOOL_g.job_queue, &job);
    pthread_mutex_unlock(&THREADPOOL_g.queue_lock); 
     
    return;
}


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
    const size_t targ)
{
    threadpool_init();
    __dependency_check(ctrl);
    __dependency_check(targ);

    // Workers is either n_workers if the stride matches exactly
    // or n_workers - 1 if it does not

    const size_t stride = tab->slice_len / THREADPOOL_g.n_workers;
   
    printf("Distributing operation over workers with stride %lu\n", stride);
 
    for (size_t i = 0; i < THREADPOOL_g.n_workers - 1; i++)
    {  
        struct distributed_tableau_op* op = (struct distributed_tableau_op*)malloc(sizeof(struct distributed_tableau_op));
        op->tab = tab;
        op->ctrl = ctrl;
        op->targ = targ;
        op->start = i * stride; 
        op->stop = (i + 1) * stride; 

        threadpool_add_task(fn, op); 
    }

    // Remainder
    struct distributed_tableau_op* op = (struct distributed_tableau_op*)malloc(sizeof(struct distributed_tableau_op));
    op->tab = tab;
    op->ctrl = ctrl;
    op->targ = targ;
    op->start = (THREADPOOL_g.n_workers - 1) * stride ; 
    op->stop = tab->slice_len;  

    threadpool_add_task(fn, op); 

    return;
}


/*
 * threadpool_worker
 * Threadpool worker function
 * :: args : void* :: Args for the worker function
 * Polls the queue and pops items from it
 * Queue items should be function pointers and associated arguments  
 * Args are NULL
 * Returns NULL
 */
void* threadpool_worker(void* args)
{
    printf("Worker created!\n");
    while (THREADPOOL_g.alive)
    {
        //printf("Acquiring queue lock for %p\n", THREADPOOL_g.job_queue);
       pthread_mutex_lock(&(THREADPOOL_g.queue_lock));
       struct threadpool_job* job = linked_list_pop(THREADPOOL_g.job_queue); 
       pthread_mutex_unlock(&(THREADPOOL_g.queue_lock));

       if (NULL != job)
       {
           printf("Acquired Job!\n"); 

           job->fn(job->args);   
           free(job->args);
           free(job);
           printf("Finished Job!\n"); 
       }
    }    
    return NULL;
} 

