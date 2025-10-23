// syswolfie.c  — xv6 x86 version
#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"
#include "string.h"

// we must declare the current process pointer
extern struct proc *proc;

// syscall: int sys_wolfie(void *buf, uint size)
int
sys_wolfie(void)
{
  char *buf;
  int size;

  // Get user arguments
  if(argptr(0, &buf, 0) < 0)
    return -1;
  if(argint(1, &size) < 0)
    return -1;

  static char *art =
    "          .--.\n"
    "         / _.-'\n"
    "    _ .-'/;\n"
    "   /_\\   /    .-\"\"-._\n"
    "  //__\\ /   .'  .--. '.\n"
    "  \\    /   /   /    \\  \\\n"
    "   '.__.'   |   |    |  |\n"
    "            |   |    |  |\n"
    "           /    \\   /   \\\n"
    "          /      '-'     \\\n";

  int len = strlen(art);
  if(size < len)
    return -1;

  // Copy art string into user buffer
  if(copyout(proc->pgdir, (uint)buf, (char *)art, len) < 0)
    return -1;

  return len;
}
