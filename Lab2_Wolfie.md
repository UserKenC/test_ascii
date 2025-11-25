# Lab2_wolfie.md

## Lab 2 – Adding a New System Call (`wolfie`)

### Overview
The goal of this lab is to extend the xv6 operating system by adding a custom system call named `wolfie`. This lab introduces the full path from kernel-level implementation to user-level invocation.

### Steps Completed

#### 1. Added System Call Number
`syscall.h`:
```c
#define SYS_wolfie 22
```

#### 2. Added Kernel Stub
`sysproc.c`:
```c
int sys_wolfie(void) {
  cprintf("wolfie!\n");
  return 0;
}
```

#### 3. Declared Prototype
`defs.h`:
```c
int sys_wolfie(void);
```

#### 4. Exposed to User Space
In `usys.S`:
```asm
SYSCALL(wolfie)
```

#### 5. Added to System Call Table
In `syscall.c`:
```c
[SYS_wolfie] sys_wolfie,
```

### 6. User Program
Created `wolfie.c`:
```c
#include "types.h"
#include "stat.h"
#include "user.h"

int main() {
  wolfie();
  exit();
}
```

### 7. Final Test
Rebuilt xv6:
```
make qemu
```
Executed:
```
$ wolfie
wolfie!
```

### Conclusion
The `wolfie` system call was successfully integrated into xv6's syscall pipeline, demonstrating understanding of kernel syscall interfaces and user-kernel interaction.
