#ifndef ASM_RTSP_R8_MSG_PROCESS_H_INCLUDE
#define ASM_RTSP_R8_MSG_PROCESS_H_INCLUDE

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include "public_def.h"

//as发给asm的as的信息
typedef struct _AS_INFO
{
    char ip[40];
    int downPort;
    int upPort;
}AS_INFO;
//sm发给asm的setup中ss的相关信息
typedef struct _S7_SS
{
    char client[128];
    char destination[40];
    int client_port;
    INT64 bandwidth;
}S7_SS;
//asm发给as的setup消息
typedef struct _R8_SETUP_MSG
{
	char as_ip[40];
	int as_port;
	int cseq;
	char require[256];
	char session_group[256];
	char ondemandsessionid[256];
	char policy[128];
	char app_id[256];
	int app_type;
	S7_SS ss;
}R8_SETUP_MSG;
//as发给asm的setup response
typedef struct _R8_SETUP_RES
{
	int err_code;
	int cseq;
    char protocol[10];
	INT64 session;
	char ondemandsessionid[128];
	S7_SS ss;
	AS_INFO as;
}R8_SETUP_RES;
//asm发给sm的teardown消息
typedef struct _R8_TEARDOWN_MSG
{
    char as_ip[40];
    int as_port;
    int cseq;
    char require[256];
    int reason;
    INT64 session;
    char ondemandsessionid[128];
}R8_TEARDOWN_MSG;
//as发给asm的teardown response
typedef struct _R8_TEARDOWN_RES
{
    int err_code;
    int cseq;
    INT64 session;
    char ondemandsessionid[128];
}R8_TEARDOWN_RES;

//解析asm向as发送的setup消息
int rtsp_r8_setup_msg_parse(char *setup, R8_SETUP_MSG *msg);
//创建as向asm发送的setup response消息
int rtsp_r8_setup_res_encode(R8_SETUP_RES res, char *setup_res);
//解析asm向as发送的teardown消息
int rtsp_r8_teardown_msg_parse(char *teardown_msg, R8_TEARDOWN_MSG *terdown);
//创建as向asm发送的teardown response消息
int rtsp_r8_teardown_res_encode(R8_TEARDOWN_RES res, char *teardown_res);

#endif
