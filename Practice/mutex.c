#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

static int globalVar = 0;
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

static void *thread1(void *arg){

    int x, j, ret;

    int loops = 10000;
    printf("\n thread 1 Executed first\n");
    ret = pthread_mutex_lock(&mtx);  // locks, thus no other thread can acces the same global variable at the same time
    if (ret != 0) perror("pthread_mutex_lock");

    for (j = 0; j < loops; j++) {
        x = globalVar;
        x++;
        globalVar = x;
    }

    ret = pthread_mutex_unlock(&mtx); // unlocks
    if (ret != 0) perror( "pthread_mutex_unlock");

    return NULL;

}

static void *thread2(void *arg){

    int y, j, ret;
    int loops = 10000;

    printf("\n thread 2| Executed first\n");
    ret = pthread_mutex_lock(&mtx);
    if (ret != 0) perror("pthread_mutex_lock");

    for (j = 0; j < loops; j++) {
        y = globalVar;
        y++;
        globalVar = y;
    }

    ret = pthread_mutex_unlock(&mtx);
    if (ret != 0) perror( "pthread_mutex_unlock");

    return NULL;
}