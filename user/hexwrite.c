#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "kernel/syscall.h"
#include "kernel/memlayout.h"
#include "kernel/riscv.h"

int get_hex(char c) {
  if(c >= '0' && c <= '9') 
    return c - '0';
  if(c >= 'A' && c <= 'F') 
    return c - 'A' + 10;
  if(c >= 'a' && c <= 'f') 
    return c - 'a' + 10;
  return -1;
}

void hexwrite(char *hexstr, char *file) {
  int count = strlen(hexstr);
  if(count == 0 || count % 2 != 0) {
    fprintf(2, "incorrect hex string format\n");
    exit(1);
  }
  count = count / 2;
  int fd = open(file, O_WRONLY);
  if(fd < 0) {
    fprintf(2, "cannot open file %s\n", file);
    exit(1);
  };
  unsigned char *buf = malloc(count);
  if(buf == 0) {
    fprintf(2, "malloc error\n");
    exit(1);
  }
  int fst, snd;
  for(int i = 0; i < count; i++) {
    fst = get_hex(hexstr[i * 2]);
    snd = get_hex(hexstr[i * 2 + 1]);
    if(fst < 0 || snd < 0) {
      fprintf(2, "invalid hex value\n");
      free(buf);
      exit(1);
    }
    buf[i] = (fst << 4) | snd;
  }

  int total_count = 0, write_count = 0;
  while(total_count < count) {
    write_count = write(fd, buf + total_count, count - total_count);
    if(write_count < 0) {
      fprintf(2, "write error\n");
      close(fd);
      free(buf);
      exit(1);
    }
    total_count += write_count;
  }
  close(fd);
  free(buf);
  if(total_count != count) {
    fprintf(2, "write byte count mismatch\n");
    exit(1);
  }
}

int main(int argc, char *argv[]) {
  if(argc != 3) {
    fprintf(2, "incorrect argument format\n");
    exit(1);
  }
  hexwrite(argv[1], argv[2]);
  exit(0);
}