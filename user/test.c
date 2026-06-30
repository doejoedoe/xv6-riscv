#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "user/user.h"

char* str_state(enum procstate state) {
  switch (state) {
  case USED: return "USED";
  case SLEEPING: return "SLEEPING";
  case RUNNABLE: return "RUNNABLE";
  case RUNNING: return "RUNNING";
  case ZOMBIE: return "ZOMBIE";
  default: return "UNUSED";
  }
}

void test_null_buf(void) {
  printf("ps_listinfo returns %d, should return process count\n", ps_listinfo(0, 0));
}

void test_invalid_addr(void) {
  int count = ps_listinfo(0, 0);
  printf("ps_listinfo returns %d, should return -3\n", ps_listinfo((struct procinfo *)-1, count * 2));
}

void test_small_buf(void) {
  int count = ps_listinfo(0, 0);
  if(--count < 1) {
    printf("too small process count to test small buffer\n");
    return;
  };
  struct procinfo* buf = malloc(count * sizeof(struct procinfo));
  if(!buf) {
    fprintf(2, "unable to allocate memory\n");
    exit(1);
  }
  printf("ps_listinfo returns %d, should return -2\n", ps_listinfo(buf, count));
  free(buf);
}

void test_big_buf(void) {
  int count = 2 * ps_listinfo(0, 0);
  struct procinfo* buf = malloc(count * sizeof(struct procinfo));
  if(!buf) {
    fprintf(2, "unable to allocate memory\n");
    exit(1);
  }
  count = ps_listinfo(buf, count);
  printf("ps_listinfo returns %d, should return process count\n", count);
  for(int i = 0; i < count; i++)
    printf("%d    %s    %s    %d    %s\n", buf[i].pid, buf[i].name, str_state(buf[i].state), buf[i].ppid, buf[i].pname);
  free(buf);
}

void test_fork(void) {
  printf("%d processes before fork\n", ps_listinfo(0, 0));
  test_big_buf();
  int pid = fork();
  if(pid == 0) {
    pause(10);
    exit(0);
  } else if(pid > 0) {
    printf("%d processes during fork\n", ps_listinfo(0, 0));
    test_big_buf();
    wait(0);
  } else {
    fprintf(2, "error during forking process\n");
    exit(1);
  }
  printf("%d processes after fork\n", ps_listinfo(0, 0));
  test_big_buf();
}

int main(void) {
  printf("\ttest null buffer\n");
  test_null_buf();
  printf("\ttest invalid address\n");
  test_invalid_addr();
  printf("\ttest small buffer\n");
  test_small_buf();
  printf("\ttest big buffer\n");
  test_big_buf();
  printf("\ttest fork\n");
  test_fork();
  exit(0);
}