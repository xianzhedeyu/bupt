#ifndef RTSP_ermi_H_INCLUDED
#define RTSP_ermi_H_INCLUDED

#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <malloc.h>

using namespace std;

typedef unsigned long long INT64;

typedef struct _QAM_INF
{
    string client;
    string bit_rate;
    string qam_id;
    string depi_mode;
}QAM_INF;

typedef struct _SETUP_MSG
{
    string rtsp_url;
    int cseq;
    string require;
    int qam_num;
    QAM_INF qaminf[5];
}SETUP_MSG;

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
}SETUP_RESPONSE;

typedef struct _TEARDOWN_MSG
{
	char rtsp_url[128];
	int cseq;
	int error_code;
	char require[128];
	char reason[128];
	INT64 session;
}TEARDOWN_MSG;

typedef struct _TEARDOWN_MSG_RES
{
	string rtsp_url;
	int cseq;
	int error_code;
	string reason;
	INT64 session;	
}TEARDOWN_MSG_RES;


typedef struct _ANNOUNCE_MSG
{
	string rtspurl;
	int cseq;
	INT64 session;
	string notice;
	string event_date;		
}ANNOUNCE_MSG;

int rtsp_ermi_setup_parse(string str,SETUP_MSG *resp_msg);

int rtsp_ermi_setup_response_encode(SETUP_RESPONSE res,char *resp_str);

int rtsp_ermi_teardown_parse(string str,TEARDOWN_MSG *resp_msg);

int rtsp_ermi_teardown_encode(TEARDOWN_MSG tear,char* resp_str);

int rtsp_ermi_teardown_res_parse(string str,TEARDOWN_MSG_RES*resp_msg);

int rtsp_ermi_teardown_res_encode(TEARDOWN_MSG_RES tear,char* resp_str);

int rtsp_ermi_announce_res_encode(ANNOUNCE_MSG ann,char* resp_str);

string rtsp_ermi_response_parse(string str);

string rtsp_ermi_error_res_encode(int error_code,int cseq);

#endif
