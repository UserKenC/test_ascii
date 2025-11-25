#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

#define MAX_HISTORY 16
#define MAX_CMD_LEN 100

#define HISTORY_FILE "/.history"

void print_history() {
    int fd = open(HISTORY_FILE, 0);
    if (fd < 0) {
        printf(2, "No history yet\n");
        return;
    }

    char buf[MAX_CMD_LEN];
    int n;
    int idx = 1;

    while ((n = read(fd, buf, MAX_CMD_LEN)) > 0) {
        int start = 0;
        for (int i = 0; i < n; i++) {
            if (buf[i] == '\n') {
                buf[i] = 0; // null-terminate this command
                printf(1, "%d %s\n", idx, &buf[start]);
                idx++;
                start = i + 1;
            }
        }
        // Handle partial command at the end of buffer
        if (start < n) {
            for (int i = start; i < n; i++)
                buf[i - start] = buf[i];
            n = n - start;
            start = 0;
        }
    }

    close(fd);
}

int main(int argc, char *argv[]) {
    print_history();
    exit();
}
