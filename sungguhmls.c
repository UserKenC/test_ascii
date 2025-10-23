#include "types.h"
#include "stat.h"
#include "user.h"

int
main(void)
{
    const int BUFSZ = 8192;      // large enough buffer
    char *buf = malloc(BUFSZ);   // allocate on heap
    if(!buf){
        printf(1, "malloc failed\n");
        exit();
    }

    int n = wolfie(buf, BUFSZ);  // pass pointer and size
    if(n < 0){
        printf(1, "wolfietest: buffer too small\n");
        free(buf);
        exit();
    }

    printf(1, "%s\n", buf);      // print ASCII art
    free(buf);
    exit();
}
