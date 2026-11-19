#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/wait.h>

#define BUF_SZ 5
#define INIT_VALUE ('C' + 'Y' / 10.0)

struct queue
{
  double buffer[BUF_SZ];
  int count;
  pthread_mutex_t lock;
  pthread_cond_t not_full;
  pthread_cond_t not_empty;
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

  // Set up the lock and conditions before fork so both processes can use them.
  pthread_mutexattr_t lock_attr;
  pthread_condattr_t cond_attr;
  if (pthread_mutexattr_init(&lock_attr) != 0 ||
      pthread_mutexattr_setpshared(&lock_attr, PTHREAD_PROCESS_SHARED) != 0 ||
      pthread_condattr_init(&cond_attr) != 0 ||
      pthread_condattr_setpshared(&cond_attr, PTHREAD_PROCESS_SHARED) != 0 ||
      pthread_mutex_init(&queue->lock, &lock_attr) != 0 ||
      pthread_cond_init(&queue->not_full, &cond_attr) != 0 ||
      pthread_cond_init(&queue->not_empty, &cond_attr) != 0)
  {
    printf("could not set up the shared queue\n");
    munmap(queue, sizeof(*queue));
    return 1;
  }
  pthread_mutexattr_destroy(&lock_attr);
  pthread_condattr_destroy(&cond_attr);
  queue->count = 0;

  pid_t p = fork();
  if (p == 0)
  {
    // Child process: generate one value at a time.
    unsigned int seed = (unsigned int)time(NULL);
    for (int k = 0; k < n; k = k + 1)
    {
      if (k > 0)
      {
        pause_ms(rand_r(&seed) % 7000);
      }
      float value = INIT_VALUE + k * d;

      pthread_mutex_lock(&queue->lock);
      while (queue->count == BUF_SZ)
      {
        pthread_cond_wait(&queue->not_full, &queue->lock);
      }
      queue->buffer[k % BUF_SZ] = value;
      queue->count = queue->count + 1;
      pthread_cond_signal(&queue->not_empty);
      pthread_mutex_unlock(&queue->lock);
    }

    munmap(queue, sizeof(*queue));
    return 0;
  }

  if (p > 0)
  {
    // Parent process: wait for a value, then print it right away.
    for (int k = 0; k < n; k = k + 1)
    {
      pthread_mutex_lock(&queue->lock);
      while (queue->count == 0)
      {
        pthread_cond_wait(&queue->not_empty, &queue->lock);
      }
      double value = queue->buffer[k % BUF_SZ];
      queue->count = queue->count - 1;
      pthread_cond_signal(&queue->not_full);
      pthread_mutex_unlock(&queue->lock);

      printf("%f\n", value);
    }
    waitpid(p, NULL, 0);
  }

  pthread_cond_destroy(&queue->not_empty);
  pthread_cond_destroy(&queue->not_full);
  pthread_mutex_destroy(&queue->lock);
  munmap(queue, sizeof(*queue));
  return 0;
}
