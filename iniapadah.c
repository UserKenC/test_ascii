#include "types.h"
#include "user.h"

int main() {
    int pid = getpid();
    printf(1, "PID %d: setting high priority 25\n", pid);
    set_priority(pid, 25);  // set high priority

    for(int i = 0; i < 100; i++){
        printf(1, "PID %d counting %d\n", pid, i);
    }

    exit();
}
