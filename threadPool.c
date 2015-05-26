//wet3 - entire file
#include "threadPool.h"
#include <stdio.h>
#include <stdlib.h>

typedef void (*funcPtr)(void *);

//writing " void (*computeFunc)(void *) " is now the same as writing " funcPtr computeFunc "
typedef struct funcAndParam {
    funcPtr function;
    void *param;
} FuncAndParam;
//task queue's member would be a pointer to this struct
//call would look like that: " function(param); "

void selfDestruct(void* x) {
    pthread_exit(NULL);
}

void* getAndExecuteTasksForever(void *voidPtrTP) {
    ThreadPool *tp = (ThreadPool *) voidPtrTP;
    while (1) {
        if (pthread_mutex_lock(&(tp->mutex_taskQueue_lock))) {//ERROORRRREE
            return NULL;
        }
        while (osIsQueueEmpty(tp->taskQueue) || tp->tpDestroyNeedsLock==1) {
            if (pthread_cond_wait(&(tp->cond_taskQueueNotEmpty), &(tp->mutex_taskQueue_lock))) {//ERROORRRREE
                return NULL;
            }
        }
        void *FAPAddress = (osDequeue(tp->taskQueue));
        if (pthread_mutex_unlock(&(tp->mutex_taskQueue_lock))) {//ERROORRRREE
            return NULL;
        }
        FuncAndParam FAP = *((FuncAndParam*)FAPAddress);
        free(FAPAddress);
        FAP.function(FAP.param);

    }

}


ThreadPool *tpCreate(int numOfThreads) {
    if (numOfThreads < 1) {//ERROORRRREE
        return NULL;
    }
    ThreadPool *tp = malloc(sizeof(*tp));
    if(!tp){
        return NULL;
    }
    tp->taskQueue = osCreateQueue();
    if (tp->taskQueue == NULL) {//ERROORRRREE
        return NULL;
    }
    tp->numOfThreads = numOfThreads;
    if (pthread_cond_init(&(tp->cond_taskQueueNotEmpty), NULL)) {//ERROORRRREE
        return NULL;
    }
    pthread_mutexattr_t mutexattr_t;
    if (pthread_mutexattr_init(&mutexattr_t)) {//ERROORRRREE
        return NULL;
    }
    if (pthread_mutexattr_settype(&mutexattr_t, PTHREAD_MUTEX_ERRORCHECK)) {//ERROORRRREE
        return NULL;
    }
    if (pthread_mutex_init(&(tp->mutex_taskQueue_lock), &mutexattr_t)) {//ERROORRRREE
        return NULL;
    }
    if (pthread_mutexattr_destroy(&mutexattr_t)) {//ERROORRRREE
        return NULL;
    }
    if (sem_init(&(tp->sem_tpDestroyWasInvoked), 0, 1)) {//ERROORRRREE
        return NULL;
    }
    tp->threadIdArray = malloc(sizeof(pthread_t)*numOfThreads);
    if(!(tp->threadIdArray)){//ERROORRRREE
        return NULL;
    }
    tp->tpDestroyNeedsLock = 0;
    int i;
    for (i = 0; i < numOfThreads; ++i) {
        pthread_t x;
        if (pthread_create(&x, NULL, getAndExecuteTasksForever, (void *) tp)) {//ERROORRRREE
            return NULL;
        }
        (tp->threadIdArray)[i] = x;
    }
    return tp;
}

void tpDestroy(ThreadPool *tp, int shouldWaitForTasks) {
    if(sem_trywait(&(tp->sem_tpDestroyWasInvoked))){//we are already during destruction
        return;
    }
    tp->tpDestroyNeedsLock = 1;
    if (pthread_mutex_lock(&(tp->mutex_taskQueue_lock))) {//ERROORRRREE
        return;//TODO think about this
    }
    if(shouldWaitForTasks==0){
        while(!(osIsQueueEmpty(tp->taskQueue))){
            free(osDequeue(tp->taskQueue));
        }
    }
    int i;
    for(i = 0 ; i < tp->numOfThreads ; ++i){
        FuncAndParam* fap = malloc(sizeof(*fap));
        fap->function = selfDestruct;
        fap->param = NULL;
        osEnqueue(tp->taskQueue, (void*)fap);
    }
    tp->tpDestroyNeedsLock = 0;
    if (pthread_cond_broadcast(&(tp->cond_taskQueueNotEmpty))) {//ERROORRRREE
        return;
    }
    if (pthread_mutex_unlock(&(tp->mutex_taskQueue_lock))) {//ERROORRRREE
        return;
    }
    int elDestroyador = getpid();//id of the process that invoked tpDestroy
    int flagCurrentIsThread = 0;//true iff the process that invoked tpDestroy is one of the threads
    for(i = 0 ; i < tp->numOfThreads ; ++i){
        if((tp->threadIdArray)[i]==elDestroyador){
            flagCurrentIsThread = 1;
        }else{
            pthread_join((tp->threadIdArray)[i], NULL);
        }
    }
    while(!(osIsQueueEmpty(tp->taskQueue))){
        free(osDequeue(tp->taskQueue));
    }
    osDestroyQueue(tp->taskQueue);
    if (pthread_cond_destroy(&(tp->cond_taskQueueNotEmpty))) {//ERROORRRREE
        return;
    }
    if (pthread_mutex_destroy(&(tp->mutex_taskQueue_lock))) {//ERROORRRREE
        return;
    }
    if (sem_destroy(&(tp->sem_tpDestroyWasInvoked))) {//ERROORRRREE
        return;
    }
    free(tp->threadIdArray);
    free(tp);
    if(flagCurrentIsThread){//if the process that invoked tpDestroy is one of the threads
        pthread_exit(NULL);
    }
}

int tpInsertTask(ThreadPool *tp, void (*computeFunc)(void *), void *param) {
    int tpDestroyWasInvoked;
    if (sem_getvalue(&(tp->sem_tpDestroyWasInvoked), &tpDestroyWasInvoked)) {//ERROORRRREE
        return -1;
    }
    if (tpDestroyWasInvoked == 0) {
        return -1;//means that tpDestroy was invoked
    }
    if (pthread_mutex_lock(&(tp->mutex_taskQueue_lock))) {//ERROORRRREE
        return -1;
    }
    if (sem_getvalue(&(tp->sem_tpDestroyWasInvoked), &tpDestroyWasInvoked)) {//ERROORRRREE
        return -1;
    }
    if (tpDestroyWasInvoked == 0) {
        return -1;//means that tpDestroy was invoked
    }


    FuncAndParam *fap = malloc(sizeof(*fap));
    if(!fap){
        return -1;
    }
    fap->function = computeFunc;
    fap->param = param;
    osEnqueue(tp->taskQueue, (void *) fap);
    if (pthread_cond_signal(&(tp->cond_taskQueueNotEmpty))) {//ERROORRRREE
        return -1;
    }
    if (pthread_mutex_unlock(&(tp->mutex_taskQueue_lock))) {//ERROORRRREE
        return -1;
    }
    return 0;
}


