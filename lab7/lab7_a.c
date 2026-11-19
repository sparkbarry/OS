#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/wait.h>

struct simulation
{
  int count;
  sem_t lock;
};

static struct simulation *shared;

static void *compute_area(void *arg)
{
  unsigned int seed = *(unsigned int *)arg;
  for (int k = 0; k < 1000000; k = k + 1)
  {
    double x = 2.0 * rand_r(&seed) / RAND_MAX - 1.0;
    double y = 2.0 * rand_r(&seed) / RAND_MAX - 1.0;

    // Only lock when the point is inside the circle.
    if (x * x + y * y <= 1.0)
    {
      while (sem_wait(&shared->lock) == -1)
      {
      }
      shared->count = shared->count + 1;
      sem_post(&shared->lock);
    }
  }
  return NULL;
}

int main(void)
{
  shared = mmap(NULL, sizeof(*shared), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  if (shared == MAP_FAILED)
  {
    return 1;
  }

  shared->count = 0;
  // &sem, pshared, value
  // pshared = 1 means shared among proc
  if (sem_init(&shared->lock, 1, 1) == -1)
  {
    munmap(shared, sizeof(*shared));
    return 1;
  }

  unsigned int seed = (unsigned int)time(NULL);
  pid_t p = fork();
  if (p < 0)
  {
    sem_destroy(&shared->lock);
    munmap(shared, sizeof(*shared));
    return 1;
  }

  // Parent uses seed and seed+1; child uses seed+2 and seed+3.
  if (p == 0)
  {
    seed = seed + 2;
  }
  unsigned int worker_seed = seed + 1;
  pthread_t thread;
  int result = 0;
  if (pthread_create(&thread, NULL, compute_area, &worker_seed) != 0)
  {
    printf("pthread_create failed\n");
    result = 1;
  }
  else
  {
    void *main_result = compute_area(&seed);
    void *worker_result;
    if (pthread_join(thread, &worker_result) != 0)
    {
      printf("pthread_join failed\n");
      _exit(1);
    }
    if (main_result != NULL || worker_result != NULL)
    {
      result = 1;
    }
  }

  if (p == 0)
  {
    munmap(shared, sizeof(*shared));
    return result;
  }

  waitpid(p, NULL, 0);

  if (result == 0)
  {
    double area = 4.0 * shared->count / (4 * 1000000);
    printf("Estimated area (pi): %.6f\n", area);
  }
  sem_destroy(&shared->lock);
  munmap(shared, sizeof(*shared));
  return result;
}
