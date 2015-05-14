//wet3 - entire file
#include "threadPool.h"
#define FALSE 0

typedef void (*funcPtr)(void *);//writing " void (*computeFunc)(void *) " is now the same as writing " funcPtr computeFunc "
typedef struct funcAndParam{
    funcPtr function;
    void* param;
}FuncAndParam;
//task queue's member would be a pointer to this struct
//call would look like that: " function(param); "

void selfDestruct(void* addressOfSelfDestructQueueMember){
    free(addressOfSelfDestructQueueMember);
    exit(0);
}
void* getAndExecuteTasksForever(void* voidPtrTP){
    ThreadPool* tp = (ThreadPool*)voidPtrTP;
    while (1) {
        if(pthread_mutex_lock(&(tp->mutex_taskQueue_lock))){//ERROORRRREE
            printf("bug1");
            return;
        }
        while(osIsQueueEmpty(tp->taskQueue)){
            if(pthread_cond_wait(&(tp->cond_taskQueueNotEmpty), &(tp->mutex_taskQueue_lock))){//ERROORRRREE
                printf("bug2");
                return;
            }
        }
        void* FAPAddress = (osDequeue(tp->taskQueue));
        if(pthread_mutex_unlock(&(tp->mutex_taskQueue_lock))){//ERROORRRREE
            printf("bug3");
            return;
        }
        FuncAndParam FAP = (FuncAndParam)(*FAPAddress);
        FAP.function(FAP.param);
    }

}


ThreadPool *tpCreate(int numOfThreads){
    if(numOfThreads<1){//ERROORRRREE
        printf("bug4");
        return NULL;
    }
    ThreadPool* tp = malloc(sizeof(*ThreadPool));
    tp->taskQueue = osCreateQueue();
    if(taskQueue == NULL){//ERROORRRREE
        printf("bug5");
        return NULL;
    }
    tp->tpDestroyInvoked = FALSE;
    tp->numOfThreads = numOfThreads;
    if(pthread_cont_init(&(tp->cond_taskQueueNotEmpty), NULL)){//ERROORRRREE
        printf("bug6");
        return NULL;
    }
    pthread_mutexattr_t mutexattr_t;
    if(pthread_mutexattr_init(&mutexattr_t)){//ERROORRRREE
        printf("bug7");
        return NULL;
    }
    if (pthread_mutexattr_setkind_np(&mutexattr_t, PTHREAD_MUTEX_ERRORCHECK)) {//ERROORRRREE
        printf("bug8");
        return NULL;
    }
    if(pthread_mutex_init(&(tp->mutex_taskQueue_lock),&mutexattr_t)){//ERROORRRREE
        printf("bug9");
        return NULL;
    }
    if(pthread_mutex_init(&(tp->mutex_Destructor),&mutexattr_t)){//ERROORRRREE
        printf("bug10");
        return NULL;
    }
    if(pthread_mutexattr_destroy(&mutexattr_t)){//ERROORRRREE
        printf("bug11");
        return NULL;
    }
    for(int i=0; i<numOfThreads; ++i){
        pthread_t x;
        if(pthread_create(&x, NULL, getAndExecuteTasksForever, (void*)tp)){//ERROORRRREE
            printf("bug12.%d",i);
            return NULL;
        }
    }
    return tp;
}

void tpDestroy(ThreadPool *threadPool, int shouldWaitForTasks){}

int tpInsertTask(ThreadPool *tp, void (*computeFunc)(void *), void *param){
    if(tp->tpDestroyInvoked){
        return -1;
    }
    if(pthread_mutex_lock(&(tp->mutex_taskQueue_lock))){//ERROORRRREE
        printf("bug13");
        return;
    }
    FuncAndParam* fap = malloc(sizeof(*fap));

}
