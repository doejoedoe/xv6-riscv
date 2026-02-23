#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  char buf[1025];
  char *s = buf;
  char c;
  uchar i = 0, flagErr = 0, flagEnd = 0, flagSpace = 0, idxSpace = 0;

  while (i < 1024 && !flagEnd && read(0, &c, 1) > 0) {
    if(c == '\n' || c == '\r') {
      *s++ = '\0';
      flagEnd = 1;
    } else {
      if(c == ' ') {
        if(!flagSpace) {
          flagSpace = 1;
          idxSpace = i;
        } else {
          flagErr = 1;
        }
      } else if(c < '0' || c > '9') {
        flagErr = 1;
      }
      *s++ = c;
      i++;
    }
  }

  printf("|%s|\n", buf);
  if(!flagSpace || !idxSpace || idxSpace + 1 == i) {
    flagErr = 1;
  }

  if(flagErr) {
    fprintf(2, "incorrect input format\n");
    exit(1);
  }

  buf[idxSpace] = '\0';
  if(!flagEnd) {
    buf[1024] = '\0';
  };

  int a, b, result;
  char *sA = buf, *sB = buf + idxSpace + 1;
  a = atoi(sA);
  b = atoi(sB);
  result = add(a, b);
  printf("%d\n", result);
  exit(0);
}