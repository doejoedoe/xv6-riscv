#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

int
main(int argc, char *argv[]) {
  char *buf = malloc(KBSIZE);
  if(!buf) {
    fprintf(2, "dmesg: uable to allocate memory\n");
    exit(1);
  }
  if(dmesg(buf) < 0) {
    fprintf(2, "dmesg: error reading buffer\n");
    free(buf);
    exit(1);
  }
  printf("%s", buf);
  free(buf);
  exit(0);
}