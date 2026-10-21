/* CS6233 Lab 5. Initials C, Y: 67 + 89 / 10.0 = 75.9. */
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

/* A pipe is a byte stream: a read/write can be interrupted or partial. */
static void transfer(int fd, void *data, size_t size, int writing) {
    char *bytes = data;
    size_t done = 0;
    while (done < size) {
        ssize_t count = writing ? write(fd, bytes + done, size - done)
                                : read(fd, bytes + done, size - done);
        if (count == -1) {
            if (errno == EINTR) continue;
            fail(writing ? "write" : "read");
        }
        if (count == 0) {
            fprintf(stderr, "Unexpected end of pipe.\n");
            exit(EXIT_FAILURE);
        }
        done += (size_t)count;
    }
}

int main(int argc, char **argv) {
    int n, channel[2];
    double d;
    parameters(argc, argv, &n, &d);
    if (pipe(channel) == -1) fail("pipe");
    pid_t child = fork();
    if (child == -1) { close(channel[0]); close(channel[1]); fail("fork"); }
    if (child == 0) {
        close(channel[0]);
        /* n and d are inherited from the parent at fork(). */
        srand((unsigned)time(NULL) ^ (unsigned)getpid());
        for (int k = 0; k < n; ++k) {
            if (k != 0) pause_ms(rand() % 7000);
            float value = (float)(INIT_VALUE + (double)k * d);
            transfer(channel[1], &value, sizeof(value), 1);
        }
        close(channel[1]);
        _exit(EXIT_SUCCESS);
    }
    close(channel[1]);
    for (int k = 0; k < n; ++k) {
        float value;
        transfer(channel[0], &value, sizeof(value), 0);
        printf("%.6f\n", (double)value);
        fflush(stdout);
    }
    close(channel[0]);
    return finish(child);
}
