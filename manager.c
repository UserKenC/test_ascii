#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include "common.h"

void perror_exit(const char *msg){
    perror(msg);
    exit(EXIT_FAILURE);
}

int main() {
    key_t shmkey = ftok(".", 'S');
    key_t semkey = ftok(".", 'M');
    if (shmkey == -1 || semkey == -1) perror_exit("ftok");

    int shmid = shmget(shmkey, sizeof(shm_buf_t), IPC_CREAT | 0600);
    if (shmid == -1) perror_exit("shmget");

    shm_buf_t *shmp = (shm_buf_t*) shmat(shmid, NULL, 0);
    if (shmp == (void*)-1) perror_exit("shmat");
    shmp->in = 0; shmp->out = 0; shmp->count = 0; shmp->next_item_id = 1;
    for (int i=0;i<BUF_SIZE;i++) shmp->buffer[i] = 0;

    int semid = semget(semkey, 3, IPC_CREAT | 0600);
    if (semid == -1) perror_exit("semget");

    union semun { int val; struct semid_ds *buf; unsigned short *array; } arg;
    arg.val = 1;
    if (semctl(semid, 0, SETVAL, arg) == -1) perror_exit("semctl mutex");
    arg.val = BUF_SIZE;
    if (semctl(semid, 1, SETVAL, arg) == -1) perror_exit("semctl empty");
    arg.val = 0;
    if (semctl(semid, 2, SETVAL, arg) == -1) perror_exit("semctl full");

    printf("Manager: created shm id %d and sem id %d\n", shmid, semid);

    pid_t pids[5];
    int idx = 0;

    for (int i=0;i<2;i++){
        pid_t pid = fork();
        if (pid < 0) perror_exit("fork");
        if (pid == 0) {
            char arg0[32], arg1[32], arg2[32];
            snprintf(arg0, sizeof(arg0), "%d", shmid);
            snprintf(arg1, sizeof(arg1), "%d", semid);
            snprintf(arg2, sizeof(arg2), "%d", i+1);
            execl("./producer", "producer", arg0, arg1, arg2, (char*)NULL);
            perror("execl producer");
            _exit(EXIT_FAILURE);
        } else {
            pids[idx++] = pid;
        }
    }

    for (int i=0;i<3;i++){
        pid_t pid = fork();
        if (pid < 0) perror_exit("fork");
        if (pid == 0) {
            char arg0[32], arg1[32], arg2[32];
            snprintf(arg0, sizeof(arg0), "%d", shmid);
            snprintf(arg1, sizeof(arg1), "%d", semid);
            snprintf(arg2, sizeof(arg2), "%d", i+1);
            execl("./consumer", "consumer", arg0, arg1, arg2, (char*)NULL);
            perror("execl consumer");
            _exit(EXIT_FAILURE);
        } else {
            pids[idx++] = pid;
        }
    }

    for (int i=0;i<5;i++){
        int status;
        pid_t w = wait(&status);
        if (w == -1) perror_exit("wait");
        printf("Manager: child %d finished with status %d\n", w, status);
    }

    if (shmdt(shmp) == -1) perror_exit("shmdt");
    if (shmctl(shmid, IPC_RMID, NULL) == -1) perror_exit("shmctl(IPC_RMID)");
    if (semctl(semid, 0, IPC_RMID) == -1) perror_exit("semctl(IPC_RMID)");

    printf("Manager: cleaned up shared memory and semaphores\n");
    return 0;
}
