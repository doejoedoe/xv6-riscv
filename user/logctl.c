#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

uint64
atouint64(const char *s)
{
  uint64 n = 0;
  while (*s >= '0' && *s <= '9') {
    n = n * 10 + (*s - '0');
    s++;
  }
  return n;
}

int
main(int argc, char *argv[]) {
  switch (argc)
  {
  case 3: {
    if(strcmp(argv[1], "off")) {
      fprintf(2, "logctl: incorrect argument format\n");
      exit(1);
    }
    uint events = atoi(argv[2]);
    logoff(events);
    break;
  }
  case 4: {
    if(strcmp(argv[1], "on")) {
      fprintf(2, "logctl: incorrect argument format\n");
      exit(1);
    }
    uint events = atoi(argv[2]);
    uint64 duration = atouint64(argv[3]);
    logon(events, duration);
    break;
  }
  default: {
    fprintf(2, "logctl: incorrect argument format\n");
    exit(1);
  }
  }
  exit(0);
}