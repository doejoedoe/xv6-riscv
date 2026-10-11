#include "param.h"
#include "types.h"
#include "memlayout.h"
#include "elf.h"
#include "riscv.h"
#include "defs.h"
#include "spinlock.h"
#include "proc.h"
#include "fs.h"

#define ENTRIES PGSIZE / sizeof(pte_t)

void str_flgs(pte_t pte, char *flgs)
{
  flgs[0] = (pte & PTE_V) ? 'V' : '_';
  flgs[1] = (pte & PTE_R) ? 'R' : '_';
  flgs[2] = (pte & PTE_W) ? 'W' : '_';
  flgs[3] = (pte & PTE_X) ? 'X' : '_';
  flgs[4] = (pte & PTE_U) ? 'U' : '_';
  flgs[5] = (pte & PTE_G) ? 'G' : '_';
  flgs[6] = (pte & PTE_A) ? 'A' : '_';
  flgs[7] = (pte & PTE_D) ? 'D' : '_';
  flgs[8] = '\0';
}

void print_px(int lvl, uint64 va)
{
  int idx = PX(lvl, va);
  if (idx < 0x10)
    printf("00%x", idx);
  else if (idx < 0x100)
    printf("0%x", idx);
  else
    printf("%x", idx);
}

void print_rec(pagetable_t pt, int lvl, uint64 va)
{
  pte_t pte;
  char flgs[9];
  uint64 child;

  for (int i = 0; i < ENTRIES; i++)
  {
    pte = pt[i];
    if (!(pte & PTE_V))
      continue;

    for (int d = 0; d < 2 - lvl; d++)
      printf(".........");

    child = va + ((uint64)i << (PGSHIFT + 9 * (lvl)));
    str_flgs(pte, flgs);
    printf(" 0x");
    print_px(lvl, child);
    printf(" -> %p %s\n", (void*)PTE2PA(pte), flgs);

    if (lvl > 0)
      print_rec((pagetable_t)PTE2PA(pte), lvl - 1, child);
  }
}

int sys_pgtable(void)
{
  struct proc *p = myproc();
  printf("PAGETABLE %p\n", p->pagetable);
  print_rec(p->pagetable, 2, 0);
  return 0;
}

uint64
sys_clrflg(void)
{
  uint64 start, end, len;
  uint flags;
  struct proc *p;
  pte_t *pte;

  argaddr(0, &start);
  argaddr(1, &len);
  argint(2, (int*)&flags);
  if (flags & ~(PTE_A | PTE_D))
    return -1;

  p = myproc();
  end = start + len;
  if (start >= p->sz || end > p->sz || end < start)
    return -1;

  start = PGROUNDDOWN(start);
  end = PGROUNDUP(end);

  for (uint64 va = start; va < end; va += PGSIZE)
  {
    pte = walk(p->pagetable, va, 0);
    if (pte && (*pte & PTE_V))
      *pte &= ~flags;
  }
  sfence_vma();
  return 0;
};

uint64
sys_getflg(void)
{
  uint64 start, end, len;
  uint flags;
  struct proc *p;
  pte_t *pte;

  argaddr(0, &start);
  argaddr(1, &len);
  argint(2, (int*)&flags);
  if (flags & ~(PTE_A | PTE_D))
    return -1;

  p = myproc();
  end = start + len;
  if (start >= p->sz || end > p->sz || end < start)
    return -1;

  start = PGROUNDDOWN(start);
  end = PGROUNDUP(end);

  for (uint64 va = start; va < end; va += PGSIZE)
  {
    pte = walk(p->pagetable, va, 0);
    if (pte && (*pte & PTE_V) && (*pte & flags))
      return 1;
  }
  return 0;
};