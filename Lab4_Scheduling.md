# Lab 4: Process Scheduling in XV6

## Overview

This lab focuses on implementing process scheduling improvements in XV6. We will introduce system calls to manipulate process priorities and retrieve statistics, and test the scheduler behavior with multiple processes.

## Lab 4 - Summary

### Objectives
- Modify the xv6 scheduler from simple round-robin to **priority-based scheduling**.  
- Assign a **priority value** to each process (0–31, where 0 is highest priority).  
- The scheduler always selects the **highest-priority process** from the ready list.  
- Implement a **system call** to change a process's priority at runtime.  
- If a process lowers its priority and another process has a higher priority in the ready list, the CPU should **switch immediately** to that higher-priority process.

---

### Tasks
1. **Implement priority scheduling**:  
   - Update `proc.c` to include a priority field.  
   - Update the scheduler to select processes based on priority.  

2. **Add system call `setpriority`**:  
   - Allows a process to change the priority of any process dynamically.  
   - Implemented in `setpriority.c` and `proc.c`.  

3. **Track scheduling statistics** (bonus task):  
   - Add fields to track **wait time, run time, and turnaround time** for each process.  
   - Create a system call `getstats` to retrieve these values.  
   - Useful for testing and visualizing scheduler behavior. 
   - Output or extract process statistics at exit.

---

## Setup Instructions
Past the following commands to set up the xv6 environment:
   ```sh
   git clone git@github.com:xiangfengyepan/xv6.git
   cd xv6
   make qemu-nox
   ```

## Implementation Details
### 1. New Files

- **pstat.h**: defines the `struct pstat` used to hold process statistics.
- **setpriority.c**: implements the system call to change process priority.
- **getstats.c**: implements the system call to retrieve process statistics.
- **setprioritytest.c**: user program to test `setpriority`.
- **getstatustest.c**: user program to test `getstats`.
- **cpuhog.c**: user program that creates a CPU-intensive process to test scheduler behavior and priority handling.

---

### 2. Process Statistics Structure (`pstat.h`)

```c
struct pstat {
  int pid;              // Process ID
  int priority;         // Current priority (lower number = higher priority)
  int wait_time;        // Time spent waiting in RUNNABLE state
  int run_time;         // Time spent running in RUNNING state  
  int turnaround_time;  // Total time from creation to termination
};
```

### 3. System Call: setpriority and getstatus

#### 3.1 setpriority
Allows a process to change the priority of another process.

File: `setpriority.c`
```c
int sys_setpriority(void)
{
  int pid, priority;
  if(argint(0, &pid) < 0 || argint(1, &priority) < 0)
    return -1;

  if(priority < MIN_PRIORITY || priority > MAX_PRIORITY)
    return -1;

  return setpriority(pid, priority); // implemented in proc.c
}
```

The main kernel logic is implemented in proc.c, which updates the target process in the process table.

#### 3.2 getstats
Used to retrieve statistics for a given process.

File: `getstats.c`
```c
int sys_getstats(void)
{
  int pid;
  struct pstat stats;
  struct pstat *user_stats;

  if(argint(0, &pid) < 0)
    return -1;
  if(argptr(1, (char**)&user_stats, sizeof(*user_stats)) < 0)
    return -1;
  if(getstats(pid, &stats) < 0)
    return -1;
  if(copyout(myproc()->pgdir, (uint)user_stats, (char*)&stats, sizeof(stats)) < 0)
    return -1;

  return 0;
}
```

The actual computation of statistics happens in `proc.c`, iterating over process states and updating wait and run times.

### 5. Testing System Calls
#### 5.1 setprioritytest.c

Tests the setpriority system call.
```c
int main(int argc, char *argv[])
{
  int pid = atoi(argv[1]);
  int priority = atoi(argv[2]);

  if(setpriority(pid, priority) < 0){
    printf(2, "setpriority failed\n");
    exit();
  }

  printf(1, "Priority of process %d set to %d\n", pid, priority);
  exit();
}
```

#### 5.2 getstatustest.c

Tests the `getstats` system call.

- Prints single-process statistics or monitors continuously.

- Displays wait time, run time, turnaround time, and current priority.

### 6. Example Scheduler Output
```sh
init: starting sh
$ 
$ ps
PID     Name    State           Prio    Wait    Run     Total
---     ----    -----           ----    ----    ---     -----
1       init    SLEEPING        16      1       5       154
2       sh      SLEEPING        16      3       2       147
3       ps      RUNNING         16      0       3       4

=== QUEUE SUMMARY ===
RUNNING:    1 process(es)
RUNNABLE:   0 process(es) in ready queue
SLEEPING:   2 process(es)
Process 3 exited: wait_time=0, run_time=5, turnaround_time=6
$ cpuhog &
Process 4 exited: wait_time=0, run_time=0, turnaround_time=0
$ ps
PID     Name    State           Prio    Wait    Run     Total
---     ----    -----           ----    ----    ---     -----
1       init    SLEEPING        16      1       5       472
2       sh      SLEEPING        16      16      5       466
6       cpuhog  RUNNABLE        31      6       123     129
5       cpuhog  SLEEPING        16      1       0       132
7       cpuhog  RUNNABLE        31      130     1       131
8       cpuhog  RUNNABLE        31      130     1       131
9       cpuhog  RUNNABLE        31      131     1       132
10      ps      RUNNING         16      0       7       7

=== QUEUE SUMMARY ===
RUNNING:    1 process(es)
RUNNABLE:   4 process(es) in ready queue - Next: PID 6 (Prio 31)
SLEEPING:   3 process(es)
Process 10 exited: wait_time=0, run_time=11, turnaround_time=11
$ setprioritytest 9 17
Priority of process 9 set to 17
Process 11 exited: wait_time=6, run_time=0, turnaround_time=6
$ ps
PID     Name    State           Prio    Wait    Run     Total
---     ----    -----           ----    ----    ---     -----
1       init    SLEEPING        16      1       5       3046
2       sh      SLEEPING        16      42      6       3039
6       cpuhog  RUNNABLE        31      593     2110    2703
5       cpuhog  SLEEPING        16      1       0       2706
7       cpuhog  RUNNABLE        31      2704    1       2705
8       cpuhog  RUNNABLE        31      2705    1       2706
9       cpuhog  RUNNABLE        17      2131    575     2706
12      ps      RUNNING         16      0       7       7

=== QUEUE SUMMARY ===
RUNNING:    1 process(es)
RUNNABLE:   4 process(es) in ready queue - Next: PID 9 (Prio 17)
SLEEPING:   3 process(es)
Process 12 exited: wait_time=0, run_time=11, turnaround_time=11
```

### 7. Notes

- Priority values: 0 (highest) to 31 (lowest), 16 (default).

- `setpriority` directly modifies the proc structure in `proc.c`.

- `getstats` calculates wait and run times for the processes in the process table.

- `ps` command reads these statistics to show the current scheduler state.