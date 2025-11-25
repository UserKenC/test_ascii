#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "syscall.h"

// System call to set process priority
int sys_setpriority(void)
{
  int pid;
  int priority;

  // Get arguments from user space
  if (argint(0, &pid) < 0 || argint(1, &priority) < 0)
    return -1;

  // Validate priority range (1-20, where 1 is highest priority)
  if (priority < MIN_PRIORITY || priority > MAX_PRIORITY)
  {
    return -1;
  }

  return setpriority(pid, priority);
}
