#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "kernel/syscall.h"
#include "kernel/memlayout.h"
#include "kernel/riscv.h"

void test_null(void) {
  int fd = open("null", O_RDWR);
  if(fd < 0) {
    fprintf(2, "cannot open null\n");
    exit(1);
  }
  char buf[8];
  printf("read from null returns %d\n", read(fd, buf, 8));
  printf("write 8 bytes to null returns %d\n", write(fd, buf, 8));
  close(fd);
}

void test_zero(void) {
  int pid = fork();
  if(pid == 0) {
    char *argv[] = {"hexdump", "4", "zero", 0};
    exec("hexdump", argv);
  } else if(pid > 0) {
    pause(10);
  } else {
    fprintf(2, "error during forking process\n");
    exit(1);
  }

  int fd = open("zero", O_RDWR);
  if(fd < 0) {
    fprintf(2, "cannot open zero\n");
    exit(1);
  }
  printf("write to zero returns %d\n", write(fd, "test", 4));
  close(fd);

  pid = fork();
  if(pid == 0) {
    char *argv[] = {"hexdump", "4", "zero", 0};
    exec("hexdump", argv);
  } else if(pid > 0) {
    pause(10);
  } else {
    fprintf(2, "error during forking process\n");
    exit(1);
  }
}

void test_urandom(void) {
  int pid = fork();
  if(pid == 0) {
    char *argv[] = {"hexdump", "4", "urandom", 0};
    exec("hexdump", argv);
  } else if(pid > 0) {
    pause(10);
  } else {
    fprintf(2, "error during forking process\n");
    exit(1);
  }

  int fd = open("urandom", O_RDWR);
  if(fd < 0) {
    fprintf(2, "cannot open urandom\n");
    exit(1);
  }
  uint64 new_seed = 0x1234567890ABCDEF;
  printf("write sizeof(uint64) bytes to urandom returns %d\n", write(fd, (char*)&new_seed, sizeof(uint64)));
  printf("write 4 bytes to urandom returns %d\n", write(fd, (char*)&new_seed, 4));
  close(fd);
  
  pid = fork();
  if(pid == 0) {
    char *argv[] = {"hexdump", "4", "urandom", 0};
    exec("hexdump", argv);
  } else if(pid > 0) {
    pause(10);
  } else {
    fprintf(2, "error during forking process\n");
    exit(1);
  }
}

void test_nullstat(void) {
  int fd = open("nullstat", O_RDWR);
  if(fd < 0) {
    fprintf(2, "cannot open nullstat\n");
    exit(1);
  }
  char buf[sizeof(uint64)];
  printf("write 4 bytes to nullstat returns %d\n", write(fd, "test", 4));
  printf("read sizeof(uint64) bytes from nullstat returns %d\n", read(fd, buf, sizeof(uint64)));
  printf("read 4 bytes from nullstat returns %d\n", read(fd, buf, 4));
  close(fd);
}

int main(void) {
  printf("\ttest null\n");
  test_null();
  printf("\ttest zero\n");
  test_zero();
  printf("\ttest urandom\n");
  test_urandom();
  printf("\ttest nullstat\n");
  test_nullstat();
  exit(0);
}