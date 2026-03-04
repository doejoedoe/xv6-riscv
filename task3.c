#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

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
    for(int i = 0; i < argc; i++) {
      if(write(pipefd[1], argv[i], strlen(argv[i])) != strlen(argv[i]) || 
        write(pipefd[1], &newline, sizeof(newline)) != sizeof(newline)) {
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
    int readCount = 0, writeCount = 0;
    while ((readCount = read(pipefd[0], buf, sizeof(buf))) > 0) {
      if((writeCount = write(STDOUT_FILENO, buf, readCount)) != readCount) {
        perror("unable to write all bytes to standart output in child");
        exit(EXIT_FAILURE);
      };
    };
    if(close(pipefd[0])) {
      perror("unable to close pipefd[0] in child");
      exit(EXIT_FAILURE);
    };
    if(readCount == -1) {
      perror("unable to read from pipefd[0] in child");
      exit(EXIT_FAILURE);
    };
    exit(EXIT_SUCCESS);
  } else {
    perror("error during forking process");
    exit(EXIT_FAILURE);
  };
}