#ifndef RTSP_S6_H_INCLUDED
#define RTSP_S6_H_INCLUDED

#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <malloc.h>

using namespace std;

typedef unsigned long long INT64;
//SM发给ERM的SETUP消息中QAM的相关信息
typedef struct _QAM_Info
{
    string client;
    string bandwidth;
    string qam_name;
    string modulation;
    //struct _QAM_Info * next;
}QAM_Info;
//SM发给ERM的SETUP消息
typedef struct _SETUP_MSG
{
    string rtsp_url;
    int cseq;
    string require;
    string session_group;
    string encryption_type;
    string cas_id;
    string encrypt_control;
    string ondemandsessionid;
    string policy;
    string inband_marker;
    int qam_num;
  QAM_Info qam_info[5];
}SETUP_MSG;
//ERM发给SM的setup response消息
typedef struct _SETUP_RESPONSE
{
	int error_code;
	int cseq;
	INT64 session;
	string client;
	string qam_destination;
	string destination;
	int client_port;
	string qam_name;
	string qam_group;
	string modulation;
	string edge_input_group;
	string embeddedEncryptor;
	string onDemandSessionId;	
}SETUP_RESPONSE;
//SM给ERM发的teardown消息
typedef struct _TEARDOWN_MSG1
{
	char rtsp_url[128];
	int cseq;
	char require[128];
	char reason[128];
	INT64 session;
	char ondemandsessionid[128];
}TEARDOWN_MSG1;
//ERM给SM发的teardown消息
typedef struct _TEARDOWN_MSG2
{
	string rtsp_url;
	int cseq;
	string reason;
	INT64 session;
	string ondemandsessionid;
}TEARDOWN_MSG2;
//SM给ERM发的teardown response
typedef struct _TEARDOWN_RES1
{
	int cseq;
	INT64 session;
	//string ondemandsessionid;
	char ondemandsessionid[128];
}TEARDOWN_RES1;
//ERM给SM发的teardown response
typedef struct _TEARDOWN_RES2
{
	int error_code;
	int cseq;
	INT64 session;
	//string ondemandsessionid;
	char ondemandsessionid[128];
}TEARDOWN_RES2;
//ERM给SM发的announce消息
typedef struct _ANNOUNCE_MSG
{
	string rtspurl;
	int cseq;
	INT64 session;
	string notice;
	string event_date;
	string npt;
	string ondemandsessionid;
}ANNOUNCE_MSG;


//解析SETUP消息
int rtsp_s6_setup_msg_parse(string str,SETUP_MSG *resp_msg);
//创建setup response消息
int rtsp_s6_setup_response_encode(SETUP_RESPONSE res,char *resp_str);
//解析SM发给ERM的teardown消息
int rtsp_s6_teardown_msg_parse(string str,TEARDOWN_MSG1 *resp_msg);
//创建ERM给SM发的teardown消息
int rtsp_s6_teardown_msg_encode(TEARDOWN_MSG2 tear,char* resp_str);
//解析SM发给ERM的teardown response消息
int rtsp_s6_teardown_res_parse(string str,TEARDOWN_RES1*resp_msg);
//创建ERM给SM发的teardown response消息
int rtsp_s6_teardown_res_encode(TEARDOWN_RES2 tear,char* resp_str);
//创建ERM给SM发的announce消息
int rtsp_s6_announce_res_encode(ANNOUNCE_MSG ann,char* resp_str);
//获取消息类型
string rtsp_s6__response_parse(string str);
//创建ERM发给SM的错误response消息
string rtsp_s6_error_res_encode(int error_code,int cseq);



#endif // RTSP_S6_H_INCLUDED
