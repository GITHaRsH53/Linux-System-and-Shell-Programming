#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
void lock_init();
void lock(int);
void unlock(int);
void *work(void *s);

int flag[2];
int turn, val = 0;

// Simple main function for implementing Peterson's Algorithm
void main()
{
    pthread_t t1, t2;
    val = 0; // shared value

    lock_init();

    // Creating threads
    pthread_create(&t1, NULL, work, (void *)0);
    pthread_create(&t2, NULL, work, (void *)1);

    // wait for the threads to join
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    // Printing the result
    printf("Final Value: %d\n", val);
}

void lock_init()
{
    // They are reset by resetting their preference to acquire locks. This is done by giving one of them a turn.
    flag[0] = flag[1] = 0;
    turn = 0;
}

// function for locking Petersons Algorithm
void lock(int other)
{
    flag[other] = 1;  // Marking this thread wants to enter the critical section
    turn = 1 - other; // Assuming that the order thread has priority
    while (flag[1 - other] == 1 && turn == 1 - other);
}

// Method for unlocking Petersonbs algorithm

void unlock(int other)
{
    flag[other] = 0; // Marking that this thread is no longer wants to enter the critical section
}

// function to know which thread is entered
void *work(void *s)
{
    int i = 0;
    int other = (int *)s;

    printf("Thread : %d\n", other);
    lock(other);

    for (i = 0; i < 100000000; i++) // 100000000 or 1e-9 means "one times ten to the negative ninth power
        val++;

    unlock(other);
}