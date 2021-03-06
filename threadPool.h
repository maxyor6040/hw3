#ifndef __THREAD_POOL__
#define __THREAD_POOL__

#include "osqueue.h"
#include <pthread.h>
#include <semaphore.h>

typedef struct thread_pool {
    //region wet3 max
    OSQueue* taskQueue;
    pthread_mutex_t mutex_taskQueue_lock;
    pthread_cond_t cond_taskQueueNotEmpty;
    int numOfThreads;
    sem_t sem_tpDestroyWasInvoked;
    pthread_t* threadIdArray;
    int tpDestroyNeedsLock;
    //endregion
} ThreadPool;

ThreadPool *tpCreate(int numOfThreads);

void tpDestroy(ThreadPool *threadPool, int shouldWaitForTasks);

int tpInsertTask(ThreadPool *threadPool, void (*computeFunc)(void *), void *param);

#endif
