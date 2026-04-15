#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"

struct spinlock rtc_lock;

void rtcinit(void)
{
  initlock(&rtc_lock, "rtc");
}

uint32
rtc_read_reg(uint64 addr)
{
  return *(volatile uint32 *)addr;
}

uint64
rtctime(void)
{
  uint32 high, low;

  acquire(&rtc_lock);
  low = rtc_read_reg(RTC_LOW);
  high = rtc_read_reg(RTC_HIGH);
  release(&rtc_lock);

  return ((uint64)high << 32) | low;
}