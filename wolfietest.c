#include "types.h"
#include "stat.h"
#include "user.h"

int
main(void)
{
  char buf[512];
  int n = wolfie(buf, sizeof(buf));
  if (n < 0)
    printf(1, "wolfie failed\n");
  else
    printf(1, "%s", buf);
  exit();
}
