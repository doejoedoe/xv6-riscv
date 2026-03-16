#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

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
  if(pipe(pipefd)) {
    fprintf(2, "error during pipe creation\n");
    exit(1);
  };
  int pid = fork();
  if(pid > 0) {
    if(close(pipefd[0])) {
      fprintf(2, "unable to close pipefd[0] in parent\n");
      exit(1);
    };
    char newline = '\n';
    long arglen, total;
    for(int i = 0; i < argc; i++) {
      arglen = strlen(argv[i]);
      total = writeAll(pipefd[1], argv[i], arglen);
      if(total == -1L || total != arglen || writeAll(pipefd[1], &newline, 1) != 1L) {
        fprintf(2, "error while writing to pipefd[1] in parent\n");
        exit(1);
      };
    };
    if(close(pipefd[1])) {
      fprintf(2, "unable to close pipefd[1] in parent\n");
      exit(1);
    };
    if(wait(0) != pid) {
      fprintf(2, "error during waiting for child in parent\n");
      exit(1);
    };
    exit(0);
  } else if(pid == 0) {
    if(close(0)) {
      fprintf(2, "unable to close standart input in child\n");
      exit(1);
    };
    if(dup(pipefd[0])) {
      fprintf(2, "unable to duplicate pipefd[0] in child\n");
      exit(1);
    };
    if(close(pipefd[0])) {
      fprintf(2, "unable to close pipefd[0] in child\n");
      exit(1);
    };
    if(close(pipefd[1])) {
      fprintf(2, "unable to close pipefd[1] in child\n");
      exit(1);
    };
    char *childArgv[] = {"/wc", 0};
    exec(childArgv[0], childArgv);
    fprintf(2, "unable to execute /wc from child\n");
    exit(1);
  } else {
    fprintf(2, "error during forking process\n");
    exit(1);
  };
}