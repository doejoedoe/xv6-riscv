#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "proc.h"
#include "fs.h"
#include "sleeplock.h"
#include "file.h"

struct file*
mutexalloc(void) {
  struct mutex* m;
  struct file *f;
  int pid = myproc()->pid;

  m = kalloc();
  if(!m) {
    printf("failure to kalloc mutex on process %d\n", pid);
    return 0;
  }
  initsleeplock(&m->lock, "mutex");
  m->pid = -1;

  
  f = filealloc();
  if(!f) {
    printf("failure to filealloc mutex on process %d\n", pid);
    kfree(m);
    return 0;
  }
  f->type = FD_MUTEX;
  f->mutex = m;

  printf("mutexalloc on process %d\n", pid);
  return f;
}

void
mutex_lock(struct mutex* m) {
  int pid = myproc()->pid;
  if(!m) panic("mutex_lock: null mutex");
  acquiresleep(&m->lock);
  m->pid = pid;

  printf("mutex_lock on process %d\n", pid);
}

void
mutex_unlock(struct mutex* m) {
  int pid = myproc()->pid;
  if(!m) panic("mutex_unlock: null mutex");
  if(m->pid != pid) 
    panic("mutex_unlock: not holding mutex");
  m->pid = -1;
  releasesleep(&m->lock);

  printf("mutex_unlock on process %d\n", pid);
}

void
mutexclose(struct mutex* m) {
  if(!m) panic("mutexclose: null mutex");
  kfree(m);

  printf("mutexclose on process %d\n", myproc()->pid);
}