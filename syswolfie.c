// syswolfie.c – xv6 x86 version
#include "types.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "memlayout.h"
#include "spinlock.h"
#include "string.h"

// syscall: int sys_wolfie(void *buf, uint size)
int
sys_wolfie(void)
{
  char *buf;
  int size;

  // Fetch syscall arguments
  if(argptr(0, &buf, 0) < 0)
    return -1;
  if(argint(1, &size) < 0)
    return -1;

  static const char *art =
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

  // Copy ASCII art to user space
  if(copyout(proc->pgdir, (uint)buf, art, len) < 0)
    return -1;

  return len; // return number of bytes copied
}
