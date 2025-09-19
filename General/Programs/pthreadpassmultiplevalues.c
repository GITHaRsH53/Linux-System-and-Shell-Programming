#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
struct arg_struct
{ // structure which contains multiple variables that are to passed as input to the thread
    int arg1;
    int arg2;
};
void *agrima(void *chinmay)
{
    struct arg_struct *args = chinmay;
    printf("%d\n", args->arg1);
    printf("%d\n", args->arg2);
    pthread_exit(NULL);
}
void *sum(void *arguments)
{
    struct arg_struct *args = arguments;
    int *results = malloc(sizeof(int));
    *results=args->arg1+args->arg2;
    printf("sum is %d\n", *results);
    pthread_exit(NULL);
}
int main()
{
    pthread_t t,t1;
    struct arg_struct naina;
    int *result;
    naina.arg1 = 5;
    naina.arg2 = 7;
    pthread_create(&t, NULL, agrima, &naina);
    // structure passed as 4th argument
    pthread_join(t, NULL); /* Wait until thread is finished */
   pthread_create(&t1, NULL, sum, &naina);
   pthread_join(t1, NULL);
}