#ifndef _PSTAT_H_
#define _PSTAT_H_

// Process statistics structure for scheduling
struct pstat {
  int pid;              // Process ID
  int priority;         // Current priority (lower number = higher priority)
  int wait_time;        // Time spent waiting in RUNNABLE state
  int run_time;         // Time spent running in RUNNING state  
  int turnaround_time;  // Total time from creation to termination
};

#endif // _PSTAT_H_