#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/wait.h>

#define BUF_SZ 5
#define INIT_VALUE ('C' + 'Y' / 10.0)

struct queue
{
  double buffer[BUF_SZ];
  sem_t empty;
  sem_t full;
};

static void pause_ms(int ms)
{
  struct timespec delay;
  delay.tv_sec = ms / 1000;
  delay.tv_nsec = (ms % 1000) * 1000000L;
  while (nanosleep(&delay, &delay) == -1)
  {
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

  struct queue *queue = mmap(NULL, sizeof(*queue), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  if (queue == MAP_FAILED)
  {
    return 1;
  }

  // The buffer starts with five empty slots and no values to read.
  if (sem_init(&queue->empty, 1, BUF_SZ) == -1)
  {
    munmap(queue, sizeof(*queue));
    return 1;
  }
  if (sem_init(&queue->full, 1, 0) == -1)
  {
    sem_destroy(&queue->empty);
    munmap(queue, sizeof(*queue));
    return 1;
  }

  pid_t p = fork();
  if (p == 0)
  {
    // Child process: take an empty slot and fill it.
    unsigned int seed = (unsigned int)time(NULL);
    for (int k = 0; k < n; k = k + 1)
    {
      if (k > 0)
      {
        pause_ms(rand_r(&seed) % 7000);
      }
      float value = INIT_VALUE + k * d;

      while (sem_wait(&queue->empty) == -1)
      {
      }
      queue->buffer[k % BUF_SZ] = value;
      sem_post(&queue->full);
    }

    munmap(queue, sizeof(*queue));
    return 0;
  }

  if (p > 0)
  {
    // Parent process: read a full slot and make it empty again.
    for (int k = 0; k < n; k = k + 1)
    {
      while (sem_wait(&queue->full) == -1)
      {
      }
      double value = queue->buffer[k % BUF_SZ];
      sem_post(&queue->empty);
      printf("%f\n", value);
    }
    waitpid(p, NULL, 0);
  }

  sem_destroy(&queue->full);
  sem_destroy(&queue->empty);
  munmap(queue, sizeof(*queue));
  return 0;
}
