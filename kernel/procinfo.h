#ifndef PROCINFO
#define PROCINFO

#ifndef PROCSTATE
#define PROCSTATE
enum procstate { UNUSED, USED, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };
#endif

struct procinfo
{
  int pid;
  char name[16];
  enum procstate state;
  int ppid;
  char pname[16];
};

#endif