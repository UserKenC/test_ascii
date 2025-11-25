#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"

void print_single_stats(int pid, struct pstat *stats)
{
  printf(1, "Prio=%d Wait=%d Run=%d Total=%d\n", stats->priority,
         stats->wait_time, stats->run_time, stats->turnaround_time);
}

void print_continuous_stats(int pid)
{
  // printf(1, "Continuous monitoring for process %d\n", pid);
  // printf(1, "Press 'q'+enter to stop, or just enter for next update\n");
  // printf(1, "Auto-stop after 100 updates\n\n");

  struct pstat prev_stats, curr_stats;

  // Get initial stats
  if (getstats(pid, &prev_stats) < 0)
  {
    printf(2, "Process %d not found\n", pid);
    return;
  }

  int count = 0;
  char buffer[10];

  while (count < 100)
  {
    if (getstats(pid, &curr_stats) < 0)
    {
      printf(2, "\nProcess %d no longer exists\n", pid);
      break;
    }

    // Calculate deltas
    int delta_wait = curr_stats.wait_time - prev_stats.wait_time;
    int delta_run = curr_stats.run_time - prev_stats.run_time;

    printf(1, "Update %d: Prio=%d Wait=%d(+%d) Run=%d(+%d) Total=%d", count,
           curr_stats.priority, curr_stats.wait_time, delta_wait,
           curr_stats.run_time, delta_run, curr_stats.turnaround_time);

    // Show state
    if (delta_run > 0)
      printf(1, " [RUNNING]");
    else if (delta_wait > 0)
      printf(1, " [WAITING]");
    else
      printf(1, " [SLEEPING]");

    printf(1, " > ");

    prev_stats = curr_stats;
    count++;

    // Check for user input
    gets(buffer, sizeof(buffer));
    if (buffer[0] == 'q' || buffer[0] == 'Q')
    {
      printf(1, "Stopping monitoring...\n");
      break;
    }
  }

  if (count >= 100)
  {
    printf(1, "Auto-stopped after 100 updates\n");
  }
}

void print_usage(char *program_name)
{
  printf(2, "Usage:\n");
  printf(2, "  %s <pid>               - Show statistics\n", program_name);
  printf(2, "  %s -c <pid>            - Continuous monitoring\n", program_name);
  printf(2, "\nExamples:\n");
  printf(2, "  %s 1        # Show stats for process 1\n", program_name);
  printf(2, "  %s -c 1     # Monitor process 1 continuously\n", program_name);
  printf(2, "\nIn continuous mode:\n");
  printf(2, "  - Press 'q'+enter to stop monitoring\n");
  printf(2, "  - Press enter to continue\n");
  printf(2, "  - Auto-stops after 100 updates\n");
}

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    print_usage(argv[0]);
    exit();
  }


  // Check for continuous mode
  if (strcmp(argv[1], "-c") == 0)
  {
    // Continuous monitoring mode
    if (argc < 3)
    {
      printf(2, "Error: PID required for continuous mode\n");
      print_usage(argv[0]);
      exit();
    }

    int pid = atoi(argv[2]);
    print_continuous_stats(pid);
  }
  else
  {
    // Single statistics mode
    int pid = atoi(argv[1]);
    struct pstat stats;

    if (getstats(pid, &stats) < 0)
    {
      printf(2, "getstats failed: process %d not found\n", pid);
      exit();
    }

    print_single_stats(pid, &stats);
  }

  exit();
}