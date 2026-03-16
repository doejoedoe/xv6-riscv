#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(void)
{
  int pid, pidSelf, pidChild, statusChild;
  pid = fork();
  if(pid > 0) {
    pidSelf = getpid();
    printf("parent pid: %d, child pid: %d\n", pidSelf, pid);
    pidChild = wait(&statusChild);
    if(pidChild == -1) {
      fprintf(2, "error during waiting for child process\n");
      exit(1);
    };
    printf("child pid: %d, exit with status: %d\n", pidChild, statusChild);
    exit(0);
  } else if(pid == 0) {
    pause(100);
    exit(1);
  } else {
    fprintf(2, "error during forking process\n");
    exit(1);
  };
}