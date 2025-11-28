#include "types.h"
#include "x86.h"
#include "pstat.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"

extern struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;

static struct proc* findproc(int pid)
{
    struct proc *p;
    // ptable.lock must be held by the caller
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
        if(p->pid == pid)
            return p;
    }
    return 0; // Not found
}


// ------------------------------------------------------------------
// EXISTING SYSTEM CALLS (Rest of the file)
// ------------------------------------------------------------------

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// sys_getprocs: user passes (struct uproc *buf, int max)
// returns number of entries copied, or -1 on error.
int
sys_getprocs(void)
{
  struct uproc *ubuf;
  int max;

  if(argint(1, &max) < 0)
    return -1;
  if(max <= 0)
    return -1;
  if(argptr(0, (char**)&ubuf, max * sizeof(struct uproc)) < 0)
    return -1;

  acquire(&ptable.lock);
  int i = 0;
  struct proc *p;
  struct uproc tmp;
  for(p = ptable.proc; p < &ptable.proc[NPROC] && i < max; p++){
    if(p->state == UNUSED)
      continue;
    tmp.pid = p->pid;
    tmp.state = p->state;
    safestrcpy(tmp.name, p->name, sizeof(tmp.name));
    // ubuf is validated by argptr, write into it
    // NOTE: This assumes 'ubuf' is correctly handled in your 'sys_getprocs' 
    // and that you have a definition for struct uproc. The argument handling
    // for argptr is non-standard for xv6 systems which use copyout to write to user space.
    // If your `argptr` is used for writing, this is fine, otherwise you need `copyout`.
    ubuf[i] = tmp; 
    i++;
  }
  release(&ptable.lock);

  return i;
}

int sys_getstats(void)
{
    int pid;
    char *stats_user_addr; // Use char* to hold the user pointer address
    int len;

    // Get the first argument (pid)
    if (argint(0, &pid) < 0)
        return -1;
    
    // Get the second argument (stats pointer) and its length (size of struct pstat)
    // The kernel implementation uses argptr to validate the address
    len = sizeof(struct pstat);
    if (argptr(1, &stats_user_addr, len) < 0)
        return -1;

    // Call the kernel implementation function
    return getstats(pid, (struct pstat *)stats_user_addr);
}

int sys_setpriority(void)
{
    int pid;
    int new_priority;

    // Get arguments from user space
    if (argint(0, &pid) < 0 || argint(1, &new_priority) < 0)
        return -1;

    // Call the kernel implementation function (defined in proc.c)
    return setpriority(pid, new_priority);
}
