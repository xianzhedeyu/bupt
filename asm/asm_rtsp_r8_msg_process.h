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
#include "asm_rtsp_s7_msg_process.h"

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

//创建asm向as发送的setup消息
int rtsp_r8_setup_msg_encode(R8_SETUP_MSG msg, char *setup_msg);
//解析as向asm发送的setup response消息
int rtsp_r8_setup_res_parse(char *setup_res_msg, R8_SETUP_RES *res);
//创建asm向as发送的teardown消息
int rtsp_r8_teardown_msg_encode(R8_TEARDOWN_MSG msg, char* teardown_msg);
//解析as发给asm的teardown response消息
int rtsp_r8_teardown_res_parse(char* teardown_res_msg, R8_TEARDOWN_RES* res);

#endif // ASM_RTSP_R8_MSG_PROCESS_H_INCLUDED
