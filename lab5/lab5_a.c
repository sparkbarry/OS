
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <sys/wait.h>
#include <stdatomic.h>
#include <sys/mman.h>

#define BUF_SZ 5
#define INIT_VALUE ('C' + 'Y' / 10.0)

// ms -> seconds and nanoseconds
void pause_ms(int ms){
    struct timespec delay;
    delay.tv_sec = ms / 1000;
    delay.tv_nsec = (ms % 1000) * 1000000L;
    while (nanosleep(&delay, &delay) == -1){
        if (errno != EINTR){
            perror("nanosleep");
            exit(1);
        }
    }
}

struct counters{
    atomic_int produced;
    atomic_int consumed;
};

int main(int argc, char* argv[]){
    if (argc != 3){
        printf("Usage: %s n d\n", argv[0]);
        return 1;
    }
    int n = atoi(argv[1]);
    double d = atof(argv[2]);
    if (n <= 1){
        printf("n must be bigger than 1\n");
        return 1;
    }

    double* buffer = mmap(NULL, BUF_SZ * sizeof(double),
                          PROT_READ | PROT_WRITE,
                          MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (buffer == MAP_FAILED){
        perror("mmap");
        return 1;
    }

    // Keep the counters separate from the five-double buffer
    struct counters* queue = mmap(NULL, sizeof(struct counters),
                                  PROT_READ | PROT_WRITE,
                                  MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (queue == MAP_FAILED){
        perror("mmap");
        munmap(buffer, BUF_SZ * sizeof(double));
        return 1;
    }
    atomic_init(&queue->produced, 0);
    atomic_init(&queue->consumed, 0);
    if (!atomic_is_lock_free(&queue->produced) ||
        !atomic_is_lock_free(&queue->consumed)){
        printf("Lock-free counters are needed\n");
        munmap(buffer, BUF_SZ * sizeof(double));
        munmap(queue, sizeof(struct counters));
        return 1;
    }

    pid_t p = fork();
    if (p == 0){
        // Child process: n and d are inherited from parent
        srand(time(NULL));
        for (int k = 0; k < n; k = k + 1){
            if (k > 0){
                pause_ms(rand() % 7000);
            }
            float value = INIT_VALUE + k * d;

            // Buffer is full when five values have not been read
            while (k - atomic_load(&queue->consumed) >= BUF_SZ){
                pause_ms(1);
            }
            buffer[k % BUF_SZ] = value;
            atomic_store(&queue->produced, k + 1);
        }
    }
    else if (p > 0){
        // Parent process: read and print each value
        for (int k = 0; k < n; k = k + 1){
            while (atomic_load(&queue->produced) == k){
                pause_ms(1);
            }
            double value = buffer[k % BUF_SZ];
            atomic_store(&queue->consumed, k + 1);
            printf("%f\n", value);
            fflush(stdout);
        }
        while (waitpid(p, NULL, 0) == -1 && errno == EINTR){}
    }
    else{
        perror("fork");
        munmap(buffer, BUF_SZ * sizeof(double));
        munmap(queue, sizeof(struct counters));
        return 1;
    }

    munmap(buffer, BUF_SZ * sizeof(double));
    munmap(queue, sizeof(struct counters));
    return 0;
}
