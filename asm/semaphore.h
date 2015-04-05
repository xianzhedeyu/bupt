#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/sem.h>

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

int set_semvalue(int sem_id);
void del_semvalue(int sem_id);
int semaphore_p(int sem_id);
int semaphore_v(int sem_id);
