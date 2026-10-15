#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "user/user.h"

#define MTRY 10
#define MPROC 1000

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

int main(int argc, char *argv[]) {
  int size = 2, try = 0, count, result;
  struct procinfo *buf = 0;
  count = ps_listinfo(buf, 0);

  if(count > 0) size = count * 2;

  while (try < MTRY) {
    buf = malloc(size * sizeof(struct procinfo));
    if(!buf) {
      fprintf(2, "unable to allocate memory\n");
      exit(1);
    }
    result = ps_listinfo(buf, size);
    if(result == -2) {
      free(buf);
      buf = 0;
      size *= 2;
      if(size > MPROC) {
        fprintf(2, "too many processes\n");
        exit(1);
      }
      try++;
      continue;
    } else if (result < 0) {
      fprintf(2, "sys call error %d\n", result);
      free(buf);
      exit(1);
    } else
      break;
  }

  if(try >= MTRY) {
    fprintf(2, "too many tries\n");
    if(buf) free(buf);
    exit(1);
  }

  printf("PID    NAME    STATE    PPID    PNAME\n");
  for(int i = 0; i < result; i++)
    printf("%d    %s    %s    %d     %s\n", buf[i].pid, buf[i].name, str_state(buf[i].state), buf[i].ppid, buf[i].pname);  
  printf("Total: %d processes\n", result);

  if(buf) free(buf);
  exit(0);
}