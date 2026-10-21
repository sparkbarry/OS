/* CS6233 Lab 5. Initials C, Y: 67 + 89 / 10.0 = 75.9. */
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#define INIT_VALUE ('C' + 'Y' / 10.0)

#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

static void fail(const char *what) { perror(what); exit(EXIT_FAILURE); }

static void pause_ms(long ms) {
    struct timespec delay = {ms / 1000, (ms % 1000) * 1000000L};
    while (nanosleep(&delay, &delay) == -1)
        if (errno != EINTR) fail("nanosleep");
}

static void parameters(int argc, char **argv, int *n, double *d) {
    char *end;
    if (argc != 3) {
        fprintf(stderr, "Usage: %s n d (integer n > 1, finite double d)\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    errno = 0;
    long count = strtol(argv[1], &end, 10);
    if (errno || end == argv[1] || *end || count <= 1 || count > INT_MAX) {
        fprintf(stderr, "Invalid n: expected an integer from 2 to INT_MAX.\n");
        exit(EXIT_FAILURE);
    }
    errno = 0;
    *d = strtod(argv[2], &end);
    if (errno || end == argv[2] || *end || !isfinite(*d)) {
        fprintf(stderr, "Invalid d: expected a finite double.\n");
        exit(EXIT_FAILURE);
    }
    *n = (int)count;
}

static int finish(pid_t child) {
    int status;
    while (waitpid(child, &status, 0) == -1)
        if (errno != EINTR) fail("waitpid");
    return WIFEXITED(status) && WEXITSTATUS(status) == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

#include <stdatomic.h>
#include <sys/mman.h>
#define BUF_SZ 5

/* Queue metadata is separate, so the data mapping is exactly 5 doubles.
   Only the child writes produced; only the parent writes consumed.
   Atomic loads/stores publish slots; there is no lock or lock acquisition. */
struct counters { atomic_uint produced; atomic_uint consumed; };

int main(int argc, char **argv) {
    int n;
    double d;
    parameters(argc, argv, &n, &d);
    double *buffer = mmap(NULL, BUF_SZ * sizeof(double), PROT_READ | PROT_WRITE,
                          MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (buffer == MAP_FAILED) fail("mmap buffer");
    struct counters *queue = mmap(NULL, sizeof(*queue), PROT_READ | PROT_WRITE,
                                  MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (queue == MAP_FAILED) { munmap(buffer, BUF_SZ * sizeof(double)); fail("mmap counters"); }
    atomic_init(&queue->produced, 0);
    atomic_init(&queue->consumed, 0);
    /* Refuse platforms where atomics could hide an internal lock. */
    if (!atomic_is_lock_free(&queue->produced) || !atomic_is_lock_free(&queue->consumed)) {
        fprintf(stderr, "This platform needs lock-free unsigned atomics.\n");
        munmap(buffer, BUF_SZ * sizeof(double));
        munmap(queue, sizeof(*queue));
        return EXIT_FAILURE;
    }
    pid_t child = fork();
    if (child == -1) {
        munmap(buffer, BUF_SZ * sizeof(double));
        munmap(queue, sizeof(*queue));
        fail("fork");
    }
    if (child == 0) {
        /* n and d are inherited from the parent at fork(). */
        srand((unsigned)time(NULL) ^ (unsigned)getpid());
        for (unsigned k = 0; k < (unsigned)n; ++k) {
            if (k != 0) pause_ms(rand() % 7000);
            /* The generated element is float, as requested; storage is double. */
            float value = (float)(INIT_VALUE + (double)k * d);
            while (k - atomic_load_explicit(&queue->consumed, memory_order_acquire) >= BUF_SZ)
                pause_ms(1); /* Sleep while full; no spin lock. */
            buffer[k % BUF_SZ] = value;
            atomic_store_explicit(&queue->produced, k + 1, memory_order_release);
        }
        munmap(buffer, BUF_SZ * sizeof(double));
        munmap(queue, sizeof(*queue));
        _exit(EXIT_SUCCESS);
    }
    for (unsigned k = 0; k < (unsigned)n; ++k) {
        while (atomic_load_explicit(&queue->produced, memory_order_acquire) == k)
            pause_ms(1); /* Sleep while empty; check about every millisecond. */
        double value = buffer[k % BUF_SZ];
        atomic_store_explicit(&queue->consumed, k + 1, memory_order_release);
        printf("%.6f\n", value);
        fflush(stdout);
    }
    int result = finish(child);
    munmap(buffer, BUF_SZ * sizeof(double));
    munmap(queue, sizeof(*queue));
    return result;
}
