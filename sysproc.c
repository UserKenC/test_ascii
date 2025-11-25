#include "types.h"
#include "x86.h"
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
    ubuf[i] = tmp;
    i++;
  }
  release(&ptable.lock);

  return i;
}
