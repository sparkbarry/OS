#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#define INIT_VALUE ('C' + 'Y' / 10.0)
#define PORT 8123
int main(int argc, char *argv[])
{
  if (argc != 3)
    return -1;

  int n = atoi(argv[1]);
  double d = atof(argv[2]);

  if (n <= 1)
    return -1;

  struct sockaddr_in address = {0};
  address.sin_port = htons(PORT);
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = htonl(0x7f000001U);
  int socket_fd;
  srand((unsigned)time(NULL));
  pid_t parent = fork();
  if (parent < 0)
    return -1;
  if (parent)
  {
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0)
      return -1;
    if (bind(socket_fd, (const struct sockaddr *)&address, sizeof(address)) < 0)
      return -1;
    usleep(1000 * (3000 + rand() % 3000));
    while (listen(socket_fd, 3) != 0)
    {
    }
    int conn_fd = accept(socket_fd, NULL, NULL);
    close(socket_fd);
    if (conn_fd < 0)
    {
      return -1;
    }
    float buffer;
    while (1)
    {
      ssize_t received = read(conn_fd, &buffer, sizeof(buffer));
      if (received <= 0)
        break;
      while (received < sizeof(buffer))
      {
        ssize_t got = read(conn_fd, (char *)&buffer + received, sizeof(buffer) - received);
        if (got == 0)
        {
          close(conn_fd);
          return -1;
        }
        if (got == -1)
        {
          if (errno == EINTR)
            continue;
          close(conn_fd);
          return -1;
        }
        received += got;
      }
      printf("%f\n", buffer);
    }
    close(conn_fd);
    if (waitpid(parent, NULL, 0) == -1)
    {
      perror("waitpid");
      return 1;
    }
  }
  else
  {
    do
    {
      socket_fd = socket(AF_INET, SOCK_STREAM, 0);
      if (socket_fd < 0)
        continue;
      if (connect(socket_fd, (const struct sockaddr *)&address, sizeof(address)) < 0)
      {
        usleep(100 * 1000);
        close(socket_fd);
        continue;
      }
      break;
    } while (1);
    // connected
    for (int k = 0; k < n; k++)
    {
      float f = INIT_VALUE + k * d;
      write(socket_fd, &f, sizeof(f));
      if (k != n - 1)
        usleep(rand() % 7000 * 1000);
    }
    close(socket_fd);
  }
  return 0;
}