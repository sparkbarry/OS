
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

#define INIT_VALUE ('C' + 'Y' / 10.0)

void pause_ms(int ms)
{
    struct timespec delay;
    delay.tv_sec = ms / 1000;
    delay.tv_nsec = (ms % 1000) * 1000000L;
    while (nanosleep(&delay, &delay) == -1)
    {
        if (errno != EINTR)
        {
            exit(1);
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("wrong params\n");
        return 1;
    }
    int n = atoi(argv[1]);
    double d = atof(argv[2]);
    if (n <= 1)
    {
        printf("n must be bigger than 1\n");
        return 1;
    }

    int fd[2];
    if (pipe(fd) == -1)
    {
        return 1;
    }

    pid_t p = fork();
    if (p == 0)
    {
        // Child process only writes to the pipe
        close(fd[0]);
        srand(time(NULL));
        for (int k = 0; k < n; k = k + 1)
        {
            if (k > 0)
            {
                pause_ms(rand() % 7000);
            }
            float value = INIT_VALUE + k * d;

            // One float fits in an atomic write to this blocking pipe
            ssize_t result;
            do
            {
                result = write(fd[1], &value, sizeof(value));
            } while (result == -1 && errno == EINTR);
            if (result != sizeof(value))
            {
                close(fd[1]);
                return 1;
            }
        }
        close(fd[1]);
    }
    else if (p > 0)
    {
        // Parent process only reads from the pipe
        close(fd[1]);
        for (int k = 0; k < n; k = k + 1)
        {
            float value;
            size_t received = 0;
            // A read can return less than one complete float
            while (received < sizeof(value))
            {
                ssize_t result = read(fd[0], (char *)&value + received,
                                      sizeof(value) - received);
                if (result == -1 && errno == EINTR)
                {
                    continue;
                }
                if (result <= 0)
                {
                    printf("Pipe read failed\n");
                    goto OUT;
                }
                received = received + result;
            }
            printf("%f\n", value);
            fflush(stdout);
        }
OUT:
        close(fd[0]);
    }
    else
    {
        close(fd[0]);
        close(fd[1]);
        return 1;
    }

    return 0;
}
