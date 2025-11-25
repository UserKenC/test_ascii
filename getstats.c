#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "pstat.h"
#include "x86.h"
#include "syscall.h"

// System call to get process scheduling statistics
int
sys_getstats(void)
{
  int pid;
  struct pstat stats;
  struct pstat *user_stats;

  // Get pid argument
  if(argint(0, &pid) < 0)
    return -1;

  // Get pointer to user's stats struct
  if(argptr(1, (char**)&user_stats, sizeof(*user_stats)) < 0)
    return -1;

  // Call the function that does the actual work
  if(getstats(pid, &stats) < 0)
    return -1;

  // Copy the results back to user space
  if(copyout(myproc()->pgdir, (uint)user_stats, (char*)&stats, sizeof(stats)) < 0)
    return -1;

  return 0;
}