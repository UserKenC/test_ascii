#include "types.h"
#include "stat.h"
#include "user.h"
#include "param.h"

int main()
{
  int i;

  for (i = 0; i < 4; i++)
  {
    if (fork() == 0)
    {
      setpriority(getpid(), MAX_PRIORITY);

      int i = 0;
      while (1)
        i++;
      exit();
    }
  }

  wait();
  exit();
}