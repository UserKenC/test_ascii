#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/time.h>
#include <time.h>
#include "common.h"

void perror_exit(const char *msg){ perror(msg); exit(EXIT_FAILURE); }

struct sembuf P(int semnum){ struct sembuf op = {semnum, -1, 0}; return op; }
struct sembuf V(int semnum){ struct sembuf op = {semnum, +1, 0}; return op; }

void print_time(){
    struct timeval tv;
    gettimeofday(&tv, NULL);
    printf("[%ld.%06ld] ", tv.tv_sec, (long)tv.tv_usec);
}

int main(int argc, char *argv[]){
    if (argc != 4){
        fprintf(stderr, "Usage: %s <shmid> <semid> <consumer_id>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int shmid = atoi(argv[1]);
    int semid = atoi(argv[2]);
    int cid = atoi(argv[3]);

    srand(time(NULL) ^ (getpid()<<16));

    shm_buf_t *shmp = (shm_buf_t*) shmat(shmid, NULL, 0);
    if (shmp == (void*)-1) perror_exit("consumer shmat");

    for (int iter=0; iter<4; iter++){
        int t = (rand() % 3) + 1;
        sleep(t);

        if (semop(semid, (struct sembuf[]){ P(2) }, 1) == -1) perror_exit("semop P full");
        if (semop(semid, (struct sembuf[]){ P(0) }, 1) == -1) perror_exit("semop P mutex");

        int item = shmp->buffer[shmp->out];
        int pos = shmp->out;
        shmp->buffer[shmp->out] = 0;
        shmp->out = (shmp->out + 1) % BUF_SIZE;
        shmp->count--;

        print_time();
        printf("Consumer %d consumed item %d from slot %d | buffer=[", cid, item, pos);
        for (int i=0;i<BUF_SIZE;i++){
            if (shmp->buffer[i]==0) printf(" _");
            else printf(" %d", shmp->buffer[i]);
        }
        printf(" ]\n");

        if (semop(semid, (struct sembuf[]){ V(0) }, 1) == -1) perror_exit("semop V mutex");
        if (semop(semid, (struct sembuf[]){ V(1) }, 1) == -1) perror_exit("semop V empty");
    }

    if (shmdt(shmp) == -1) perror_exit("consumer shmdt");
    return 0;
}
