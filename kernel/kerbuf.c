#include "kerbuf.h"
#include "types.h"
#include "param.h"
#include "riscv.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "printf.h"
#include "proc.h"
#include "defs.h"
#include "syscall.h"

extern struct spinlock wait_lock;
struct kernel_buffer ker_buf;
static struct spinlock log_lock;
static uint32 log_events = 0;
static uint64 tick_duration = 0;

void ker_buf_init()
{
  initlock(&ker_buf.lock, "ker_buf.lock");
  initlock(&log_lock, "log_lock");
  ker_buf.data[0] = '\n';
  ker_buf.count = 1;
  ker_buf.head = 0;
  ker_buf.tail = 1;
}

void kb_putc(char c)
{
  acquire(&ker_buf.lock);
  if(ker_buf.count >= KBSIZE) {
    ker_buf.head = (ker_buf.head + 1) % KBSIZE;
    ker_buf.count--;
  }
  ker_buf.data[ker_buf.tail] = c;
  ker_buf.tail = (ker_buf.tail + 1) % KBSIZE;
  ker_buf.count++;
  release(&ker_buf.lock);
}

static void
kb_printint(long long xx, int base, int sgn)
{
  char buf[20];
  int i, neg;
  unsigned long long x;

  neg = 0;
  if(sgn && xx < 0){
    neg = 1;
    x = -xx;
  } else {
    x = xx;
  }

  i = 0;
  do{
    buf[i++] = "0123456789abcdef"[x % base];
  }while((x /= base) != 0);
  if(neg)
    buf[i++] = '-';

  while(--i >= 0)
    kb_putc(buf[i]);
}

static void
kb_printptr(uint64 x) {
  int i;
  kb_putc('0');
  kb_putc('x');
  for (i = 0; i < (sizeof(uint64) * 2); i++, x <<= 4)
    kb_putc("0123456789abcdef"[x >> (sizeof(uint64) * 8 - 4)]);
}

void pr_msg(const char *fmt, ...)
{
  va_list ap;
  char* s;
  int c0, c1, c2, i, state;

  acquire(&tickslock);
  uint ticks_val = ticks;
  release(&tickslock);

  kb_putc('[');
  kb_printint(ticks_val, 10, 0);
  kb_putc(']');
  kb_putc(' ');

  va_start(ap, fmt);

  state = 0;
  for(i = 0; fmt[i]; i++){
    c0 = fmt[i] & 0xff;
    if(state == 0){
      if(c0 == '%'){
        state = '%';
      } else {
        kb_putc(c0);
      }
    } else if(state == '%'){
      c1 = c2 = 0;
      if(c0) c1 = fmt[i+1] & 0xff;
      if(c1) c2 = fmt[i+2] & 0xff;
      if(c0 == 'd'){
        kb_printint(va_arg(ap, int), 10, 1);
      } else if(c0 == 'l' && c1 == 'd'){
        kb_printint(va_arg(ap, uint64), 10, 1);
        i += 1;
      } else if(c0 == 'l' && c1 == 'l' && c2 == 'd'){
        kb_printint(va_arg(ap, uint64), 10, 1);
        i += 2;
      } else if(c0 == 'u'){
        kb_printint(va_arg(ap, uint32), 10, 0);
      } else if(c0 == 'l' && c1 == 'u'){
        kb_printint(va_arg(ap, uint64), 10, 0);
        i += 1;
      } else if(c0 == 'l' && c1 == 'l' && c2 == 'u'){
        kb_printint(va_arg(ap, uint64), 10, 0);
        i += 2;
      } else if(c0 == 'x'){
        kb_printint(va_arg(ap, uint32), 16, 0);
      } else if(c0 == 'l' && c1 == 'x'){
        kb_printint(va_arg(ap, uint64), 16, 0);
        i += 1;
      } else if(c0 == 'l' && c1 == 'l' && c2 == 'x'){
        kb_printint(va_arg(ap, uint64), 16, 0);
        i += 2;
      } else if(c0 == 'p'){
        kb_printptr(va_arg(ap, uint64));
      } else if(c0 == 'c'){
        kb_putc(va_arg(ap, uint32));
      } else if(c0 == 's'){
        if((s = va_arg(ap, char*)) == 0)
          s = "(null)";
        for(; *s; s++)
          kb_putc(*s);
      } else if(c0 == '%'){
        kb_putc('%');
      } else {
        kb_putc('%');
        kb_putc(c0);
      }
      state = 0;
    }
  }
  va_end(ap);
  kb_putc('\n');
}

void kb_timer_tick()
{
  acquire(&log_lock);
  if(tick_duration) {
    tick_duration--;
    if(!tick_duration)
      log_events = 0;
  }
  release(&log_lock);
}

void kb_log_event(enum events type, ...) {
  if(!(log_events & type)) return;

  va_list ap;
  va_start(ap, type);

  switch(type) {
  case SYSCALL: {
    struct proc *p = va_arg(ap, struct proc*);
    const int num = va_arg(ap, int);
    const char* call = va_arg(ap, char*);
    if(!p) {
      pr_msg("SYSCALL: null proc pointer");
      va_end(ap);
      return;
    }
    int pid;
    char name[16];
    acquire(&p->lock);
    pid = p->pid;
    safestrcpy(name, p->name, sizeof(name));
    release(&p->lock);
    pr_msg("SYSCALL: pid=%d, name=%s, call=%s number=%d", pid, name, call, num);
    break;
  }
  case INTERRUPT: {
    const int irq = va_arg(ap, int);
    const char *device = va_arg(ap, const char*);
    const char *info = va_arg(ap, const char*);
    pr_msg("INTERRUPT: irq=%d, device=%s, info=%s", irq, device, info);
    break;
  }
  case PROCSTART: {
    struct proc *p = va_arg(ap, struct proc*);
    if(!p) {
      pr_msg("PROCSTART: null proc pointer");
      va_end(ap);
      return;
    }
    
    int pid, ppid = -1;
    char name[16], pname[16] = "";
    int held_wait = holding(&wait_lock);
    int held_p = holding(&p->lock);
    if(!held_wait)
      acquire(&wait_lock);
    if(!held_p)
      acquire(&p->lock);
    pid = p->pid;
    safestrcpy(name, p->name, sizeof(name));
    if(p->parent){
      ppid = p->parent->pid;
      safestrcpy(pname, p->parent->name, sizeof(pname));
    }
    if(!held_p)
      release(&p->lock);
    if(!held_wait)
      release(&wait_lock);

    pr_msg("PROCSTART: ppid=%d, pname=%s, pid=%d, name=%s", ppid, pname, pid, name);
    break;
  }
  case PROCEND: {
    struct proc *p = va_arg(ap, struct proc*);
    if(!p) {
      pr_msg("PROCEND: null proc pointer");
      va_end(ap);
      return;
    }

    int pid, ppid = -1;
    char name[16], pname[16] = "";
    int held_wait = holding(&wait_lock);
    int held_p = holding(&p->lock);
    if(!held_wait)
      acquire(&wait_lock);
    if(!held_p)
      acquire(&p->lock);
    pid = p->pid;
    safestrcpy(name, p->name, sizeof(name));
    if(p->parent){
      ppid = p->parent->pid;
      safestrcpy(pname, p->parent->name, sizeof(pname));
    }
    if(!held_p)
      release(&p->lock);
    if(!held_wait)
      release(&wait_lock);

    pr_msg("PROCEND: ppid=%d, pname=%s, pid=%d, name=%s", ppid, pname, pid, name);
    break;
  }
  case EXEC: {
    const char *path = va_arg(ap, const char*);
    struct proc *p = va_arg(ap, struct proc*);

    if(!p) {
      pr_msg("EXEC: null proc pointer");
      va_end(ap);
      return;
    }

    int pid;
    char name[16];
    int held_p = holding(&p->lock);
    if(!held_p)
      acquire(&p->lock);
    pid = p->pid;
    safestrcpy(name, p->name, sizeof(name));
    if(!held_p)
      release(&p->lock);

    pr_msg("EXEC: pid=%d, name=%s, path=%s", pid, name, path);
    break;
  }
  default: {
    pr_msg("invalid event type");
    break;
  }
  }
  va_end(ap);
}

uint64
sys_dmesg(void) {
  uint64 user_buf;
  struct proc *p = myproc();
  uint start, copy_count, end_count;
  char null = 0;

  argaddr(0, &user_buf);
  acquire(&ker_buf.lock);

  start = ker_buf.head;
  for(uint i = 0; i < ker_buf.count; i++) {
    uint pos = (ker_buf.head + i) % KBSIZE;
    if (ker_buf.data[pos] == '\n') {
      start = (pos + 1) % KBSIZE;
      break;
    }
  }

  copy_count = (ker_buf.tail - start + KBSIZE) % KBSIZE;

  if (!copy_count) {
    release(&ker_buf.lock);
    if(copyout(p->pagetable, user_buf, &null, 1) < 0)
      return -1;
    return 0;
  }

  if(start <= ker_buf.tail) {
    if (copyout(p->pagetable, user_buf, ker_buf.data + start, copy_count) < 0) {
      release(&ker_buf.lock);
      return -1;
    }
  } else {
    end_count = KBSIZE - start;
    if (copyout(p->pagetable, user_buf, ker_buf.data + start, end_count) < 0) {
      release(&ker_buf.lock);
      return -1;
    }
    if (copyout(p->pagetable, user_buf + end_count, ker_buf.data, copy_count - end_count) < 0) {
      release(&ker_buf.lock);
      return -1;
    }
  }
  release(&ker_buf.lock);
  if(copyout(p->pagetable, user_buf + copy_count, &null, 1) < 0)
    return -1;
  return 0;
}

uint64
sys_logon(void)
{
  int events;
  uint64 duration;

  argint(0, &events);
  argaddr(1, &duration);
  acquire(&log_lock);
  log_events |= events;
  if(duration)
    tick_duration = duration;
  release(&log_lock);
  return 0;
}

uint64
sys_logoff(void)
{
  int events;

  argint(0, &events);
  acquire(&log_lock);
  log_events &= ~events;
  release(&log_lock);
  return 0;
}