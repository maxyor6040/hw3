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
            printf("bug1");
            return NULL;
        }
        while (osIsQueueEmpty(tp->taskQueue)) {
            if (pthread_cond_wait(&(tp->cond_taskQueueNotEmpty), &(tp->mutex_taskQueue_lock))) {//ERROORRRREE
                printf("bug2");
                return NULL;
            }
        }
        void *FAPAddress = (osDequeue(tp->taskQueue));
        if (pthread_mutex_unlock(&(tp->mutex_taskQueue_lock))) {//ERROORRRREE
            printf("bug3");
            return NULL;
        }
        FuncAndParam FAP = *((FuncAndParam*)FAPAddress);
        FAP.function(FAP.param);
        free(FAPAddress);

    }

}


ThreadPool *tpCreate(int numOfThreads) {
    if (numOfThreads < 1) {//ERROORRRREE
        printf("bug4");
        return NULL;
    }
    ThreadPool *tp = malloc(sizeof(*tp));
    tp->taskQueue = osCreateQueue();
    if (tp->taskQueue == NULL) {//ERROORRRREE
        printf("bug5");
        return NULL;
    }
    tp->numOfThreads = numOfThreads;
    if (pthread_cond_init(&(tp->cond_taskQueueNotEmpty), NULL)) {//ERROORRRREE
        printf("bug6");
        return NULL;
    }
    pthread_mutexattr_t mutexattr_t;
    if (pthread_mutexattr_init(&mutexattr_t)) {//ERROORRRREE
        printf("bug7");
        return NULL;
    }
    if (pthread_mutexattr_settype(&mutexattr_t, PTHREAD_MUTEX_ERRORCHECK)) {//ERROORRRREE
        printf("bug8");
        return NULL;
    }
    if (pthread_mutex_init(&(tp->mutex_taskQueue_lock), &mutexattr_t)) {//ERROORRRREE
        printf("bug9");
        return NULL;
    }
    if (pthread_mutexattr_destroy(&mutexattr_t)) {//ERROORRRREE
        printf("bug10");
        return NULL;
    }
    if (sem_init(&(tp->sem_tpDestroyWasInvoked), 0, 1)) {//ERROORRRREE
        printf("bug11");
        return NULL;
    }
    tp->threadIdArray = malloc(sizeof(pthread_t)*numOfThreads);
    if(!(tp->threadIdArray)){//ERROORRRREE
        printf("bug11.5");
        return NULL;
    }
    int i;
    for (i = 0; i < numOfThreads; ++i) {
        pthread_t x;
        if (pthread_create(&x, NULL, getAndExecuteTasksForever, (void *) tp)) {//ERROORRRREE
            printf("bug12.%d", i);
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
    if (pthread_mutex_lock(&(tp->mutex_taskQueue_lock))) {//ERROORRRREE
        printf("bug18");
        return;//TODO think about this
    }
    if(shouldWaitForTasks==0){
        while(!(osIsQueueEmpty(tp->taskQueue))){
            free(osDequeue(tp->taskQueue));
        }

    }
    int i;
    for(i = 0 ; i < tp->numOfThreads ; ++i){
        FuncAndParam fap;
        fap.function = selfDestruct;
        osEnqueue(tp->taskQueue, &fap);
        if (pthread_cond_signal(&(tp->cond_taskQueueNotEmpty))) {//ERROORRRREE
            printf("bug20");
            return -1;//TODO think about this
        }
    }
    if (pthread_mutex_unlock(&(tp->mutex_taskQueue_lock))) {//ERROORRRREE
        printf("bug19");
        return;//TODO think about this
    }

    for(i = 0 ; i < tp->numOfThreads ; ++i){
        pthread_join((tp->threadIdArray)[i], NULL);
    }
    free(tp);

}

int tpInsertTask(ThreadPool *tp, void (*computeFunc)(void *), void *param) {
    int tpDestroyWasInvoked;
    if (sem_getvalue(&(tp->sem_tpDestroyWasInvoked), &tpDestroyWasInvoked)) {//ERROORRRREE
        printf("bug14");
        return -1;//TODO think about this
    }
    if (tpDestroyWasInvoked == 0) {
        return -1;//means that tpDestroy was invoked
    }
    if (pthread_mutex_lock(&(tp->mutex_taskQueue_lock))) {//ERROORRRREE
        printf("bug13");
        return -1;//TODO think about this
    }
    FuncAndParam *fap = malloc(sizeof(*fap));
    fap->function = computeFunc;
    fap->param = param;
    osEnqueue(tp->taskQueue, (void *) fap);
    if (pthread_cond_signal(&(tp->cond_taskQueueNotEmpty))) {//ERROORRRREE
        printf("bug15");
        return -1;//TODO think about this
    }
    if (pthread_mutex_unlock(&(tp->mutex_taskQueue_lock))) {//ERROORRRREE
        printf("bug16");
        return -1;//TODO think about this
    }
    return 0;
}


