#ifndef SM_RTSP_S6_H
#define SM_RTSP_S6_H

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


//SM发给ERM的SETUP消息中QAM的相关信息
typedef struct _QAM_INFO
{
    INT64 bandwidth;
    char qam_name[128];
    char client[128];
    char modulation[128];
}QAM_INFO;
//SM发给ERM的SETUP消息
typedef struct _S6_SETUP_MSG
{
    char ip[40];
    int cseq;
    char require[256];
    char session_group[256];
    char encryption_type[128];
    char cas_id[128];
    char encrypt_control[256];
    char ondemandsessionid[256];
    char policy[128];
    char inband_marker[256];
    QAM_INFO qam_info[QAM_NUM_MAX];
    int qam_num;
}S6_SETUP_MSG;

//ERM发给SM的SETUP消息中QAM的相关信息(仅悲观模式下使用)
typedef struct _QAM_Info1
{
    char destination[40];
    int client_port;
    char client[128];
}QAM_Info1;
//ERM发给SM的setup消息（仅悲观模式下使用）
typedef struct _S6_SETUP_MSG1
{
	//char rtsp_ip[16];
	int cseq;
	char require[256];
	char session_group[256];
	char ondemandsessionid[256];
	char policy[128];
	QAM_Info1 qam_info1[QAM_NUM_MAX];
	int qam_num;
}S6_SETUP_MSG1;
//SM发给ERM的setup response消息(仅悲观模式下使用)
typedef struct _S6_SETUP_RES1
{
	int err_code;
	int cseq;
	INT64 session;
	char destination[40];
	int client_port;
	char source[40];
	int server_port;
	char client[20];
	char embedded_encryptor[10];
	char ondemandsessionid[256];	
}S6_SETUP_RES1;
//ERM发给SM的setup response消息
typedef struct _S6_SETUP_RES
{
	int err_code;
	int cseq;
	INT64 session;
	char client[20];
    char destination[40];
    int client_port;
	char qam_destination[30];
    char qam_name[50];
	char qam_group[50];
	char modulation[20];
	char edge_input_group[50];
	char embedded_encryptor[10];
	char ondemandsessionid[256];	
}S6_SETUP_RES;
//SM给ERM发的teardown消息
typedef struct _S6_TEARDOWN_MSG1
{
	char rtsp_ip[40];
	int cseq;
	char require[128];
	int reason;
	INT64 session;
	char ondemandsessionid[128];
}S6_TEARDOWN_MSG1;
//ERM给SM发的teardown消息
typedef struct _S6_TEARDOWN_MSG2
{
	char rtsp_ip[40];
	int cseq;
	char require[128];
	int reason;
	INT64 session;
	char ondemandsessionid[128];
}S6_TEARDOWN_MSG2;
//SM给ERM发的teardown response
typedef struct _S6_TEARDOWN_RES1
{
	int err_code;
	int cseq;
	INT64 session;
	char ondemandsessionid[128];
}S6_TEARDOWN_RES1;
//ERM给SM发的teardown response
typedef struct _S6_TEARDOWN_RES2
{
	int err_code;
	int cseq;
	INT64 session;
	char ondemandsessionid[128];
}S6_TEARDOWN_RES2;
//ERM给SM发的announce消息
typedef struct _S6_ANNOUNCE_MSG
{
	char rtsp_ip[40];
	int cseq;
	char require[128];
	INT64 session;
	int notice;
	char event_date[30];
	char npt[30];
	char ondemandsessionid[128];
}S6_ANNOUNCE_MSG;
//SM给ERM发的ANNOUNCE RESPONSE
typedef struct _S6_ANNOUNCE_RES
{
	int err_code;
	int cseq;
	INT64 session;
	char ondemandsessionid[128];
}S6_ANNOUNCE_RES;


//积极模式下或悲观模式下创建SM发给ERM的SETUP信息
int rtsp_s6_setup_msg_encode(S6_SETUP_MSG msg,char *setup_msg);
//悲观模式下解析ERM作为RTSP代理向SM发送的SETUP消息
int rtsp_s6_pes_setup_msg_parse(char *setup_msg,S6_SETUP_MSG1 *msg1);
//悲观模式下SM向ERM发送的Response消息
int rtsp_s6_pes_setup_res_encode(S6_SETUP_RES1 res1,char *res_msg);
//积极模式下或悲观模式下解析ERM向SM发送的Response消息
int rtsp_s6_setup_res_parse(char *msg,S6_SETUP_RES *res);
//创建SM向ERM发送的TEARDOWN消息
int rtsp_s6_teardown_msg_encode(S6_TEARDOWN_MSG1 msg1,char *tear_msg);
//解析ERM作为代理向SM发送的TEARDOWN消息
int rtsp_s6_teardown_msg_parse(char *tear_msg,S6_TEARDOWN_MSG2 *msg2);
//创建SM发给ERM的Teardown Response
int rtsp_s6_teardown_res_encode(S6_TEARDOWN_RES1 res1,char *tear_res);
//解析ERM发给SM的Teardown Response
int rtsp_s6_teardown_res_parse(char *tear_res,S6_TEARDOWN_RES2 *res2);
//解析ERM向SM发送的ANNOUNCE消息
int rtsp_s6_announce_msg_parse(char * announce,S6_ANNOUNCE_MSG *ann);
//创建SM向ERM发送的ANNOUNCE RESPONSE消息
int rtsp_s6_announce_res_encode(S6_ANNOUNCE_RES res,char *ann_res);

#endif
