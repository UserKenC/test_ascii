#ifndef COMMON_H
#define COMMON_H

#include <sys/types.h>

#define BUF_SIZE 3

typedef struct {
    int buffer[BUF_SIZE];
    int in;
    int out;
    int count;
    int next_item_id;
} shm_buf_t;

#endif
