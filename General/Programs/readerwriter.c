#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

int shared_data = 0;
int readers_count = 0;

sem_t mutex, rw_mutex;

void *reader(void *arg) {
    int id = *(int *)arg;
    while (1) {
        sem_wait(&mutex);
        readers_count++;
        if (readers_count == 1) {
            sem_wait(&rw_mutex); // First reader locks writer out
        }
        sem_post(&mutex);
        
        // Reading section
        printf("Reader %d read shared data: %d\n", id, shared_data);
        
        sem_wait(&mutex);
        readers_count--;
        if (readers_count == 0) {
            sem_post(&rw_mutex); // Last reader releases writer
        }
        sem_post(&mutex);
        
        sleep(1); // Simulate some processing time
    }
    return NULL;
}

void *writer(void *arg) {
    int id = *(int *)arg;
    while (1) {
        sem_wait(&rw_mutex);
        
        // Writing section
        shared_data++;
        printf("Writer %d wrote shared data: %d\n", id, shared_data);
        
        sem_post(&rw_mutex);
        
        sleep(2); // Simulate some processing time
    }
    return NULL;
}

int main() {
    pthread_t readers[3], writers[2];
    int reader_ids[3] = {1, 2, 3};
    int writer_ids[2] = {1, 2};
    
    // Initialize semaphores
    sem_init(&mutex, 0, 1);
    sem_init(&rw_mutex, 0, 1);
    
    // Create reader threads
    for (int i = 0; i < 3; i++) {
        pthread_create(&readers[i], NULL, reader, &reader_ids[i]);
    }
    
    // Create writer threads
    for (int i = 0; i < 2; i++) {
        pthread_create(&writers[i], NULL, writer, &writer_ids[i]);
    }
    
    // Wait for threads (though they run indefinitely in this example)
    for (int i = 0; i < 3; i++) {
        pthread_join(readers[i], NULL);
    }
    for (int i = 0; i < 2; i++) {
        pthread_join(writers[i], NULL);
    }
    
    // Clean up
    sem_destroy(&mutex);
    sem_destroy(&rw_mutex);
    
    return 0;
}