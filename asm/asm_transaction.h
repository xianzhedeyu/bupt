#include"asm_def.h"
#include "public_def.h"
#include "asm_task_control_module.h"
int Asm_Setup(char * Asm_msg,int msg_len,int asm_sd);
int Asm_Teardown(char * Asm_msg,int msg_len,int asm_sd);
int Asm_GetParam(char * Asm_msg,int msg_len,int asm_sd);
int pthread_Asm_TP(char * Asm_msg,int msg_len,int asm_sd);
