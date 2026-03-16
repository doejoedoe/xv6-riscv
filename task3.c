#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

long writeAll(int fd, const char* buf, unsigned long len)
{
  unsigned long counter = 0, total = 0;
  while(total < len) {
    counter = write(fd, buf + total, len - total);
    if(counter < 0) {
      return -1;
    };
    total += counter;
  };
  return total;
};

int
main(int argc, char *argv[])
{
  int pipefd[2];
  if(pipe(pipefd) == -1) {
    perror("error during pipe creation");
    exit(EXIT_FAILURE);
  };
  pid_t pid = fork();
  if(pid > 0) {
    if(close(pipefd[0]) == -1) {
      perror("unable to close pipefd[0] in parent");
      exit(EXIT_FAILURE);
    };
    char newline = '\n';
    long arglen, total;
    for(int i = 0; i < argc; i++) {
      arglen = strlen(argv[i]);
      total = writeAll(pipefd[1], argv[i], arglen);
      if(total == -1L || total != arglen || writeAll(pipefd[1], &newline, 1) != 1L) {
        perror("error while writing to pipefd[1] in parent");
        exit(EXIT_FAILURE);
      };
    };
    if(close(pipefd[1]) == -1) {
      perror("unable to close pipefd[1] in parent");
      exit(EXIT_FAILURE);
    };
    if(wait(0) == -1) {
      perror("error during waiting for child in parent");
      exit(EXIT_FAILURE);
    };
    exit(EXIT_SUCCESS);
  } else if(pid == 0) {
    if(close(pipefd[1]) == -1) {
      perror("unable to close pipefd[1] in child");
      exit(EXIT_FAILURE);
    };
    char buf[1024];
    long readCount = 0, writeCount = 0;
    while ((readCount = read(pipefd[0], buf, sizeof(buf))) > 0) {
      writeCount = writeAll(STDOUT_FILENO, buf, readCount);
      if(writeCount == -1L || writeCount != readCount) {
        perror("unable to write all bytes to standart output in child");
        exit(EXIT_FAILURE);
      };
    };
    if(readCount < 0) {
      perror("unable to read from pipefd[0] in child");
      exit(EXIT_FAILURE);
    };
    if(close(pipefd[0])) {
      perror("unable to close pipefd[0] in child");
      exit(EXIT_FAILURE);
    };
    exit(EXIT_SUCCESS);
  } else {
    perror("error during forking process");
    exit(EXIT_FAILURE);
  };
}