#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

// Copies an ASCII art image to a
// user-supplied buffer, provided that
// the buffer is large enough. If the
// buffer is too small, or not valid,
// return a negative value. If the call
// succeeds, return the number of bytes
// copied.
int sys_wolfie(void)
{
  static const char wolfie[] = "          .     .  .      .\n"
                               "     _     )\\.-.  )\\.-.  )\\.-.\n"
                               "    ( `-._/  `-'_/  `-'_/  `-'\n"
                               "     `--.__.-.__.-.__.-.__.-'\n"
                               "          /     _     _     \\\n"
                               "         /      (o)   (o)    \\\n"
                               "        (     .--.     .--.   )\n"
                               "         \\   (    \\___/    ) /\n"
                               "          '.  '._       _.' .'\n"
                               "            '-.__\"-----\"__.-'\n"
                               "                 ~  WOLFIE  ~\n";

  char *buf;
  uint size;
  uint bytes_copied = sizeof(wolfie) - 1;

  struct proc *curproc = myproc();

  // get syscall args from user space
  if (argptr(0, &buf, sizeof(buf)) < 0 || argint(1, (int *)&size) < 0)
    return -1;

  // Validate buffer
  if (!buf || size < bytes_copied)
    return -1;
  if ((uint)buf >= curproc->sz || (uint)buf + bytes_copied > curproc->sz)
    return -1;

  // Copy ASCII art to user buffer
  memmove(buf, wolfie, bytes_copied);

  return bytes_copied;
}