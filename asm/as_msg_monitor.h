#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>
#include "hash.h"
#include "public_def.h"
#include "asm_vrep_d8_msg_process.h"

typedef struct Argument {
    int connfd;
    char ip[40];
}Argument;
void as_msg_process();
