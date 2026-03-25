#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main() {
  int m = mutex();
  if(m < 0) {
    fprintf(2, "mutex creation failure\n");
    exit(1);
  }
  char buf[10];
  struct stat st;

  printf("\ttest 1: read/write/fstat\n"
    "read() from mutex returns %d\n"
    "write() to mutex returns %d\n"
    "fstat() of mutex returns %d\n", 
    read(m, buf, 1), 
    write(m, "x", 1),
    fstat(m, &st));

  printf("\ttest 2: close after lock\n"
    "mutex_lock returns %d\n"
    "mutex_close returns %d\n",
    mutex_lock(m),
    mutex_close(m));

  printf("\ttest 3: child locks mutex, parent closes it\n");
  m = mutex();
  if(m < 0) {
    fprintf(2, "mutex creation failure\n");
    exit(1);
  }
  int pid = fork();
  if(pid == 0) {
    printf("child %d mutex_lock returns %d\n", getpid(), mutex_lock(m));
    pause(10);
    printf("child %d mutex_close returns %d\n", getpid(), mutex_close(m));
    exit(0);
  } if(pid > 0) {
    pause(5);
    printf("parent %d mutex_close returns %d\n", getpid(), mutex_close(m));
    wait(0);
  } else {
    fprintf(2, "error during forking process\n");
    exit(1);
  }

  printf("\ttest 4: child locks mutex, parent tries to unlock it\n");
  m = mutex();
  if(m < 0) {
    fprintf(2, "mutex creation failure\n");
    exit(1);
  }
  pid = fork();
  if(pid == 0) {
    printf("child %d mutex_lock returns %d\n", getpid(), mutex_lock(m));
    pause(10);
    printf("child %d mutex_unlock returns %d\n", getpid(), mutex_unlock(m));
    exit(0);
  } if(pid > 0) {
    pause(5);
    printf("parent %d mutex_unlock returns %d\n", getpid(), mutex_unlock(m));
    wait(0);
  } else {
    fprintf(2, "error during forking process\n");
    exit(1);
  }


  printf("\ttest 5: mutex locked before forking\n");
  m = mutex();
  if(m < 0) {
    fprintf(2, "mutex creation failure\n");
    exit(1);
  }
  printf("process %d mutex_lock returns %d\n", getpid(), mutex_lock(m));
  pid = fork();
  if(pid == 0) {
    pause(10);
    printf("child %d mutex_unlock returns %d\n", getpid(), mutex_unlock(m));
    exit(0);
  } if(pid > 0) {
    pause(5);
    printf("parent %d mutex_unlock returns %d\n", getpid(), mutex_unlock(m));
    wait(0);
  } else {
    fprintf(2, "error during forking process\n");
    exit(1);
  }
  exit(0);
}