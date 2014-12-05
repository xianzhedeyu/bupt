#ifndef ASM_RTSP_S7_MSG_PROCESS_H_INCLUDED
#define ASM_RTSP_S7_MSG_PROCESS_H_INCLUDED

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

typedef struct _AS_INFO
{
	char as_ip[40];
	int as_port;
}AS_INFO;

typedef struct _S7_SS
{
	char client[128];
	char destination[40];
	int client_port;
}S7_SS;

typedef struct _S7_SETUP_MSG
{
	char asm_ip[40];
	int asm_port;
	int cseq;
	char require[256];
	char session_group[256];
	char ondemandsessionid[256];
	char policy[128];
	char app_id[256];
	int app_type;
	S7_SS ss;
}S7_SETUP_MSG;

typedef struct _S7_SETUP_RES
{
	int err_code;
	int cseq;
	INT64 session;
	char ondemand_session_id[128];
	S7_SS ss;
	AS_INFO as;
}S7_SETUP_RES;

//创建asm向sm发送的setup response消息
int rtsp_s7_setup_res_encode(S7_SETUP_RES resmsg,char *setup_res_msg);
//解析sm发送给asm的setup消息
int rtsp_s7_setup_msg_parse(char *setup_msg, S7_SETUP_MSG msg);

#endif

