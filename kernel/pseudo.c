#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"
#include "proc.h"

struct urandom_state {
  uint64 seed;
  struct spinlock lock;
};

struct nullstat_state {
  uint64 bytes_written;
  struct spinlock lock;
};

static struct urandom_state urandom;
static struct nullstat_state nullstat;

int nullread(uint64 addr, int n) {
  if(n < 0) 
    return -1;
  return 0;
}

int nullwrite(uint64 addr, int n) {
  if(n < 0) 
    return -1;
  return n;
}

int zeroread(uint64 addr, int n) {
  if(n < 0) 
    return -1;
  char zero = 0;
  for(int i = 0; i < n; i++)
    if(copyout(myproc()->pagetable, addr + i, &zero, 1) < 0)
      return -1;
  return n;
}

int zerowrite(uint64 addr, int n) {
  return -1;
}

int urandomread(uint64 addr, int n) {
  if(n < 0) 
    return -1;
  acquire(&urandom.lock);
  for(int i = 0; i < n; i++) {
    urandom.seed = urandom.seed * 10619863 + 17977;
    char byte = urandom.seed >> 16;
    if(copyout(myproc()->pagetable, addr + i, &byte, 1) < 0) {
      release(&urandom.lock);
      return -1;
    }
  }
  release(&urandom.lock);
  return n;
}

int urandomwrite(uint64 addr, int n) {
  if(n != sizeof(uint64)) return -1;
  uint64 new_seed;
  if(copyin(myproc()->pagetable, (char*)&new_seed, addr, sizeof(uint64)) < 0)
    return -1;
  acquire(&urandom.lock);
  urandom.seed = new_seed;
  release(&urandom.lock);
  return n;
}

int nullstatread(uint64 addr, int n) {
  if(n != sizeof(uint64)) 
    return -1;
  acquire(&nullstat.lock);
  uint64 count = nullstat.bytes_written;
  release(&nullstat.lock);
  return count;
}

int nullstatwrite(uint64 addr, int n) {
  if(n < 0) 
    return -1;
  acquire(&nullstat.lock);
  nullstat.bytes_written += n;
  release(&nullstat.lock);
  return n;
}

int pseudoread(int dev, uint64 addr, int n) {
  switch(minor(dev)) {
    case MINOR_NULL:
      return nullread(addr, n);
    case MINOR_ZERO:
      return zeroread(addr, n);
    case MINOR_URANDOM:
      return urandomread(addr, n);
    case MINOR_NULLSTAT:
      return nullstatread(addr, n);
    default:
      return -1;
  }
}

int pseudowrite(int dev, uint64 addr, int n) {
  switch(minor(dev)) {
    case MINOR_NULL:
      return nullwrite(addr, n);
    case MINOR_ZERO:
      return zerowrite(addr, n);
    case MINOR_URANDOM:
      return urandomwrite(addr, n);
    case MINOR_NULLSTAT:
      return nullstatwrite(addr, n);
    default:
      return -1;
  }
}

void pseudoinit(void) {
  urandom.seed = 0x9e3779b97f4a7c15;
  initlock(&urandom.lock, "urandom");
  
  nullstat.bytes_written = 0;
  initlock(&nullstat.lock, "nullstat");

  devsw[PSEUDO].read = pseudoread;
  devsw[PSEUDO].write = pseudowrite;
}
