#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"

int main(int argc, char *argv[])
{
  if (argc != 3)
  {
    printf(2, "Usage: setpriority_test <pid> <priority>\n");
    exit();
  }

  int pid = atoi(argv[1]);
  int priority = atoi(argv[2]);

  if (setpriority(pid, priority) < 0)
  {
    printf(2, "setpriority failed\n");
    exit();
  }

  printf(1, "Priority of process %d set to %d\n", pid, priority);
  exit();
}