#include"public_def.h"
#include "sm_task_control_module.h"
int Tp_Setup(char * S1_msg,int msg_len,int OC_sd,pthread_args *p_args);
int Tp_Teardown(char * S1_msg,int msg_len,int OC_sd,pthread_args *p_args);
int pthread_TP(char * S1_msg,int msg_len,int OC_sd,pthread_args *p_args);
