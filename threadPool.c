//wet3 - entire file
#include "threadPool.h"

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
void getAndExecuteTasksForever(ThreadPool *tp){
    while (1) {
        if(pthread_mutex_lock(&(tp->mutex_taskQueue_lock))){//ERROORRRREE
            printf("bug1");
            return;
        }
        while(osIsQueueEmpty(&(tp->taskQueue))){
            if(pthread_cond_wait(&(tp->cond_taskQueueNotEmpty), &(tp->mutex_taskQueue_lock))){//ERROORRRREE
                printf("bug2");
                return;
            }
        }
        void* FAPAddress = (osDequeue(&(tp->taskQueue)));
        if(pthread_mutex_unlock(&(tp->mutex_taskQueue_lock))){//ERROORRRREE
            printf("bug3");
            return;
        }
        FuncAndParam FAP = (FuncAndParam)(*FAPAddress);
        FAP.function(FAP.param);
    }

}


ThreadPool *tpCreate(int numOfThreads){}

void tpDestroy(ThreadPool *threadPool, int shouldWaitForTasks){}

int tpInsertTask(ThreadPool *threadPool, void (*computeFunc)(void *), void *param){}
