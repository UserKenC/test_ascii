#include "types.h"
#include "user.h"
#include "stat.h"
#include "fcntl.h"
#include "pstat.h"

int main(int argc, char *argv[])
{
  struct pstat stats;
  int runnable_count = 0, running_count = 0, sleeping_count = 0;
  int next_pid = -1, next_prio = 32;

  printf(1, "PID\tName\tState\t\tPrio\tWait\tRun\tTotal\n");
  printf(1, "---\t----\t-----\t\t----\t----\t---\t-----\n");

  struct proc
  {
    int pid;
    char name[16];
    int state;
  } procs[64];

  int n = getprocs(procs, 64);

  for (int i = 0; i < n; i++)
  {
    if (procs[i].state == 0)
      continue; // Skip UNUSED

    char *state;
    switch (procs[i].state)
    {
    case 1:
      state = "EMBRYO   ";
      break;
    case 2:
      state = "SLEEPING ";
      break;
    case 3:
      state = "RUNNABLE ";
      break;
    case 4:
      state = "RUNNING  ";
      break;
    case 5:
      state = "ZOMBIE   ";
      break;
    default:
      state = "?        ";
      break;
    }

    // Count states for summary
    if (procs[i].state == 3)
    { // RUNNABLE
      runnable_count++;
    }
    else if (procs[i].state == 4)
    { // RUNNING
      running_count++;
    }
    else if (procs[i].state == 2)
    { // SLEEPING
      sleeping_count++;
    }

    // Get scheduling statistics
    if (getstats(procs[i].pid, &stats) == 0)
    {
      printf(1, "%d\t%s\t%s\t%d\t%d\t%d\t%d\n", procs[i].pid, procs[i].name,
             state, stats.priority, stats.wait_time, stats.run_time,
             stats.turnaround_time);

      // Track highest priority runnable process
      if (procs[i].state == 3 && stats.priority < next_prio)
      {
        next_pid = procs[i].pid;
        next_prio = stats.priority;
      }
    }
    else
    {
      printf(1, "%d\t%s\t%s\t?\t?\t?\t?\n", procs[i].pid, procs[i].name, state);
    }
  }

  // Print queue summary
  printf(1, "\n=== QUEUE SUMMARY ===\n");
  printf(1, "RUNNING:    %d process(es)\n", running_count);
  printf(1, "RUNNABLE:   %d process(es) in ready queue", runnable_count);
  if (next_pid != -1)
  {
    printf(1, " - Next: PID %d (Prio %d)", next_pid, next_prio);
  }
  printf(1, "\n");
  printf(1, "SLEEPING:   %d process(es)\n", sleeping_count);

  exit();
}