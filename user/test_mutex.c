#include "kernel/types.h"
#include "user/user.h"

void test_print(int pid, int argc, char** argv) {
  for(int i = 0; i < argc; i++)
    for(int j = 0; argv[i][j] != 0; j++)
      printf("%d: arg %d, char '%c'\n", pid, i, argv[i][j]);
}

int main(int argc, char *argv[]) {
  int pid;
  int m;
    
  printf("\ttest without mutex\n");
  pid = fork();
  if(pid == 0) {
    test_print(getpid(), argc, argv);
    exit(0);
  } else if(pid > 0) {
    test_print(getpid(), argc, argv);
    wait(0);
  } else {
    fprintf(2, "error during forking process\n");
    exit(1);
  }
    
  printf("\ttest with mutex\n");
  m = mutex();
  if(m < 0) {
    fprintf(2, "mutex creation failure\n");
    exit(1);
  }
  pid = fork();
  if(pid == 0) {
    mutex_lock(m);
    test_print(getpid(), argc, argv);
    mutex_unlock(m);
    exit(0);
  } else if(pid > 0) {
    mutex_lock(m);
    test_print(getpid(), argc, argv);
    mutex_unlock(m);
    wait(0);
  } else {
    fprintf(2, "error during forking process\n");
    exit(1);
  }
  exit(0);
}