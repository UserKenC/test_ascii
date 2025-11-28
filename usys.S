#include "syscall.h"
#include "traps.h"

#define SYSCALL(name) \
  .globl name; \
  name: \
    movl $SYS_ ## name, %eax; \
    int $T_SYSCALL; \
    ret

SYSCALL(fork)
SYSCALL(exit)
SYSCALL(wait)
SYSCALL(pipe)
SYSCALL(read)
SYSCALL(write)
SYSCALL(close)
SYSCALL(kill)
SYSCALL(exec)
SYSCALL(open)
SYSCALL(mknod)
SYSCALL(unlink)
SYSCALL(fstat)
SYSCALL(link)
SYSCALL(mkdir)
SYSCALL(chdir)
SYSCALL(dup)
SYSCALL(getpid)
SYSCALL(sbrk)
SYSCALL(sleep)
SYSCALL(uptime)
SYSCALL(wolfie)
SYSCALL(getprocs)
SYSCALL(getstats)
SYSCALL(setpriority)

// Generic syscall stub for ulib.c functions
.global syscall
syscall:
    // Arguments are typically passed on the stack in C, and the first argument
    // (the syscall number) is at 4(%esp).
    // We move the syscall number into %eax.
    movl 4(%esp), %eax
    
    // We move the remaining arguments (pid, priority, etc.) onto the stack
    // so the kernel can find them easily, often done by copying the
    // remaining stack frame, but a simple xv6 implementation might just
    // expect the arguments to remain on the stack after the syscall number
    // has been popped/used. 
    
    // Simplest approach: Just trigger the interrupt
    int $T_SYSCALL
    ret
