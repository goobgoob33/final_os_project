// thread_version.c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <semaphore.h>

int matrix[1000][1000];
long long shared = 0;
sem_t my_semaphore;

void* count_ones(void* arg) {
    int thread_id = *(int*)arg;
    int start_row = thread_id;
    int local_count = 0;
    
    printf("Thread %d starting work!\n", thread_id);
    
    for(int j = 0; j < 1000; j++) {
        if(matrix[start_row][j] == 1) {
            local_count++;
        }
    }
    
    printf("Thread %d found %d ones, waiting for semaphore...\n", thread_id, local_count);
    sem_wait(&my_semaphore);
    shared += local_count;
    sem_post(&my_semaphore);
    printf("Thread %d finished adding its count!\n", thread_id);
    
    return NULL;
}

int main() {
    srand(time(NULL));
    printf("start!\n");
    
    for(int i = 0; i < 1000; i++) {
        for(int j = 0; j < 1000; j++) {
            matrix[i][j] = rand() % 2;
        }
    }
    
    sem_init(&my_semaphore, 0, 1);
    
    pthread_t threads[1000];
    int thread_ids[1000];
    
    clock_t start = clock();  // start timing!
    
    for(int i = 0; i < 1000; i++) {
        thread_ids[i] = i;
        if(pthread_create(&threads[i], NULL, count_ones, &thread_ids[i]) != 0) {
            printf("failed to create thread :(\n");
            return 1;
        }
    }
    
    for(int i = 0; i < 1000; i++) {
        pthread_join(threads[i], NULL);
    }
    
    clock_t end = clock();
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("total lines: %lld\n", shared);
    printf("time: %f seconds\n", time_spent);
    
    sem_destroy(&my_semaphore);
    return 0;
}