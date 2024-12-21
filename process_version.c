// process_version.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>

#define PIPE_NAME "/tmp/my_pipe"

int main() {
    int matrix[1000][1000];
    
    srand(time(NULL));
    printf("Starting the program!\n");
    
    printf("Creating random matrix...\n");
    for(int i = 0; i < 1000; i++) {
        for(int j = 0; j < 1000; j++) {
            matrix[i][j] = rand() % 2;
        }
    }
    mkfifo(PIPE_NAME, 0666);
    printf("Created pipe!\n");
    
    clock_t start = clock();
    
    printf("Creating processes...\n");
    for(int i = 0; i < 1000; i++) {
        pid_t pid = fork();
        
        if(pid == 0) { 
            int my_count = 0;
            for(int j = 0; j < 1000; j++) {
                if(matrix[j][i] == 1) { 
                    my_count++;
                }
            }
            
            printf("process %d found %d ones.\n", i, my_count);
            
            int pipe_fd = open(PIPE_NAME, O_WRONLY);
            write(pipe_fd, &my_count, sizeof(int));
            close(pipe_fd);
            
            exit(0);
        }
    }
    
    int pipe_fd = open(PIPE_NAME, O_RDONLY);
    long long total = 0;
    int temp;
    
    for(int i = 0; i < 1000; i++) {
        read(pipe_fd, &temp, sizeof(int));
        total += temp;
    }
    
    for(int i = 0; i < 1000; i++) {
        wait(NULL);
    }
    
    clock_t end = clock();
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("total lines: %lld\n", total);
    printf("time: %f seconds\n", time_spent);
    
    close(pipe_fd);
    unlink(PIPE_NAME);
    return 0;
}