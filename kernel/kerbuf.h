#ifndef KERBUF
#define KERBUF
#include "types.h"
#include "spinlock.h"
#include "param.h"

struct kernel_buffer {
  struct spinlock lock;
  char data[KBSIZE];
  uint head;
  uint tail;
  uint32 count;
};

enum events { 
  SYSCALL = 1, 
  INTERRUPT = 1 << 1, 
  PROCSTART = 1 << 2, 
  PROCEND = 1 << 3, 
  EXEC = 1 << 4 
};

extern struct kernel_buffer ker_buf;

void ker_buf_init();

void kb_putc(char c);
char kb_getc();
void pr_msg(const char* fmt, ...);

void kb_log_event(enum events type, ...);
void kb_timer_tick();

uint64 sys_dmesg(void);
uint64 sys_logon(void);
uint64 sys_logoff(void);

#endif