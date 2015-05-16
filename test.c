#include <stdio.h>
#include <stdlib.h>
#include "osqueue.h"
#include "threadPool.h"


void hello(void *a) {
    //printf("hello\n");
    double i;
    for(i=0; i<100000; i+=0.0025);
}


void test_thread_pool_sanity() {
    int i;

    ThreadPool *tp = tpCreate(5);

    for (i = 0; i < 100; ++i) {
        tpInsertTask(tp, hello, NULL);
    }

    tpDestroy(tp, 1);
}


int main() {
    test_thread_pool_sanity();

    return 0;
}
