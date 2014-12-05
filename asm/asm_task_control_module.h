#ifndef _erm_task_control_module_h
#define _erm_task_control_module_h
#include"asm_queuemanager.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include<signal.h>
#include <sys/wait.h>
#include<unistd.h>
#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>

#include <sys/mman.h>
#include <string.h>
 #include <stropts.h>

//#pragma pack(1)
#define LOCKFILE "./lockfile"
#define LOCKDB "./lockdb"
#define LOCKMODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)
#define MMAPDIR "/home/orion/workspaces/c/erm/mmapdir"
#define write_lock(fd) lockfile((fd),F_WRLCK)
#define write_unlock(fd) lockfile((fd),F_UNLCK)
typedef void Sigfunc(int);
typedef struct {
	int flag;/*标识该段内存数据是否为空*/
	char *mmap_queue;/*sizeof(int)+sizeof(SOCKET_DATA)*/
}mmap_queue_info;
typedef struct {
	char *Queue_addr;/*动态共享内存段节点指针*/
}Queue_info;

typedef struct{
	int data_id;/*共享内存区的数据编号*/
	int data_flag;
	int pthread_flag;
	int sd;
	int data_len;
	char data[1024];
}pthread_queue;
/*记录进程pid和对应的节点指针，在进程异常退出时负责标记*/
typedef struct{
	int pid;/*进程pid*/
	char *pid_queue_addr;/*共享内存节点指针*/
}process_info;

int lockfile(int fd,int type);
int interproxy(int lockfd);
int process_manager(int fr,int sr,int lockfd);
void *pthread_R6(void *arg);
void *pthread_Ermi(void *arg);
#endif
