#include "types.h"
#include "stat.h"
#include "user.h"

void print_test_message(int pid, int prio) {
    printf(1, "Process %d with priority %d is running.\n", pid, prio);
}

int main() {
    int pid1, pid2;

    // First child
    pid1 = fork();
    if (pid1 == 0) {
        set_priority(getpid(), 5);
        print_test_message(getpid(), 5);
        exit();
    }

    // Only parent forks second child
    if (pid1 > 0) {
        pid2 = fork();
        if (pid2 == 0) {
            set_priority(getpid(), 10);
            print_test_message(getpid(), 10);
            exit();
        }
    }

    // Parent waits for children
    wait();
    wait();

    printf(1, "Priority test complete.\n");
    exit();
}
