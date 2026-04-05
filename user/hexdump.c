#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "kernel/syscall.h"
#include "kernel/memlayout.h"
#include "kernel/riscv.h"

void hexdump(int count, char *file) {
  int fd = open(file, O_RDONLY);
  if(fd < 0) {
    fprintf(2, "cannot open file %s\n", file);
    exit(1);
  }
  unsigned char buf[16];
  int total_count = 0, to_read = 0, read_count = 0;
  while(total_count < count) {
    to_read = count - total_count;
    if(to_read > 16) 
      to_read = 16;    
    read_count = read(fd, buf, to_read);
    if(read_count < 0) {
      fprintf(2, "read error\n");
      close(fd);
      exit(1);
    }
    for(int i = 0; i < read_count; i++) {
      if(buf[i] < 0x10) {
        printf("0");
      }
      printf("%x", buf[i]);
      if(total_count + i < count - 1) 
        printf(" ");
    }
    total_count += read_count;
  }
  printf("\n");
  close(fd);
  if(total_count != count) {
    fprintf(2, "read byte count mismatch\n");
    exit(1);
  };
}

int main(int argc, char *argv[]) {
  if(argc != 3) {
    fprintf(2, "incorrect argument format\n");
    exit(1);
  }
  int count = atoi(argv[1]);
  if(count <= 0) {
    fprintf(2, "count must be positive\n");
      exit(1);
  }
  hexdump(count, argv[2]);
  exit(0);
}