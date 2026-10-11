#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define PTE_A (1L << 6)
#define PTE_D (1L << 7)
#define PGROUNDDOWN(a) (((a)) & ~(PGSIZE-1))
#define PGROUNDUP(sz) (((sz)+PGSIZE-1) & ~(PGSIZE-1))
#define PGSIZE 4096
#define ARRSIZE 10

int var_gl = 42;
int arr_gl[ARRSIZE] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

void where(void *ptr, char *name) {
  printf("%s at %p (page round down 0x%lx, up 0x%lx)\n", name, ptr, PGROUNDDOWN((uint64)ptr), PGROUNDUP((uint64)ptr));
}

void getflgs(int* var_st_addr, int* arr_st_addr, int* arr_heap_addr) {
  printf("global var flag A: %d D: %d\t", getflg((uint64)&var_gl, sizeof(var_gl), PTE_A), getflg((uint64)&var_gl, sizeof(var_gl), PTE_D));
  printf("global arr flag A: %d D: %d\t", getflg((uint64)arr_gl, sizeof(arr_gl), PTE_A), getflg((uint64)arr_gl, sizeof(arr_gl), PTE_D));
  printf("stack var flag A: %d D: %d\t", getflg((uint64)var_st_addr, sizeof(*var_st_addr), PTE_A), getflg((uint64)var_st_addr, sizeof(*var_st_addr), PTE_D));
  printf("stack arr flag A: %d D: %d\t", getflg((uint64)arr_st_addr, ARRSIZE * sizeof(int), PTE_A), getflg((uint64)arr_st_addr, ARRSIZE * sizeof(int), PTE_D));
  printf("heap arr flag A: %d D: %d\n", getflg((uint64)arr_heap_addr, PGSIZE * 4, PTE_A), getflg((uint64)arr_heap_addr, PGSIZE * 4, PTE_D));
}

int main(void)
{
  printf("\tpagetable before tests\n");
  pgtable();

  int var_st = 24;
  int arr_st[ARRSIZE] = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1};

  int* arr_heap = (int*)malloc(PGSIZE * 4);
  if(!arr_heap) {
    fprintf(2, "unable to allocate memory\n");
    exit(1);
  }

  where(&var_gl, "var_gl");
  where(arr_gl, "arr_gl_beg");
  where(&arr_gl[ARRSIZE - 1], "arr_gl_end");
  where(&var_st, "var_st");
  where(arr_st, "arr_st_beg");
  where(&arr_st[ARRSIZE - 1], "arr_st_end");
  where(arr_heap, "arr_heap_beg");
  where(&arr_heap[PGSIZE * 4 - 1], "arr_heap_end");

  printf("\tpagetable after malloc\n");
  pgtable();

  printf("\tgetting flags\n");
  getflgs(&var_st, arr_st, arr_heap);

  for(int i = 0; i < PGSIZE * 4 / sizeof(int); i++)
    arr_heap[i] = i;
  printf("\tflags after writing to heap array\n");
  getflgs(&var_st, arr_st, arr_heap);
  printf("\tpagetable after writing to heap array\n");
  pgtable();

  clrflg((uint64)&var_gl, sizeof(var_gl), PTE_A | PTE_D);
  clrflg((uint64)&var_st, sizeof(var_st), PTE_A | PTE_D);
  clrflg((uint64)arr_gl, sizeof(arr_gl), PTE_A | PTE_D);
  clrflg((uint64)arr_st, sizeof(arr_st), PTE_A | PTE_D);
  clrflg((uint64)arr_heap, PGSIZE * 4, PTE_A | PTE_D);
  printf("\tflags after after clearing flags flag A | D\n");
  getflgs(&var_st, arr_st, arr_heap);
  printf("\tpagetable after clearing flags A | D\n");
  pgtable();

  int sum = var_gl;
  sum += arr_gl[0] + arr_gl[9];
  sum += var_st;
  sum += arr_st[0] + arr_st[9];
  for(int i = 0; i < PGSIZE * 4 / sizeof(int); i++)
    sum += arr_heap[i];
  printf("\tflags after reading from all vars and arrs\n");
  getflgs(&var_st, arr_st, arr_heap);
  printf("\tpagetable after reading from all vars and arrs\n");
  pgtable();

  var_gl *= 10;
  arr_gl[0] *= 10;
  arr_gl[ARRSIZE - 1] *= 10;
  var_st *= 10;
  arr_st[0] *= 10;
  arr_st[ARRSIZE - 1] *= 10;
  for(int i = 0; i < PGSIZE * 4 / sizeof(int); i++)
    arr_heap[i] *= 10;
  printf("\tflags after writing to all vars and arrs\n");
  getflgs(&var_st, arr_st, arr_heap);
  printf("\tpagetable after writing to all vars and arrs\n");
  pgtable();

  free(arr_heap);
  printf("\tpagetable after freeing heap arr\n");
  pgtable();
  exit(0);
}