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

typedef struct _EQAM_INF
{
    string client;
    string bit_rate;
    string qam_id;
    string depi_mode;
}EQAM_INF;

typedef struct _ESETUP_MSG
{
    string rtsp_url;
    int cseq;
    string require;
    int qam_num;
    EQAM_INF qaminf[5];
}ESETUP_MSG;

typedef struct _ESETUP_RESPONSE
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
	string depi_mode;
	string taps;
	string increment;
	string j_83;
	string channelwidth;
}ESETUP_RESPONSE;

typedef struct _ETEARDOWN_MSG
{
	char rtsp_url[128];
	int cseq;
	char require[128];
	INT64 session;
}ETEARDOWN_MSG;

typedef struct _ETEARDOWN_MSG_RES
{
	string rtsp_url;
	int cseq;
	int error_code;
	INT64 session;	
}ETEARDOWN_MSG_RES;

typedef struct _EGETPARAM_MSG
{
	string rtsp_url;
	int cseq;
	INT64 session;
	int clen;
	string ctype;
	string param;
	string require;
}EGETPARAM_MSG;

typedef struct _EANNOUNCE_MSG
{
	string rtspurl;
	int cseq;
	INT64 session;
	string notice;
	string event_date;		
}EANNOUNCE_MSG;

int rtsp_ermi_setup_parse(string str,ESETUP_MSG *resp_msg);

int rtsp_ermi_setup_response_encode(ESETUP_RESPONSE &res,char *resp_str);

int rtsp_ermi_teardown_parse(string str,ETEARDOWN_MSG *resp_msg);

int rtsp_ermi_teardown_encode(ETEARDOWN_MSG tear,char* resp_str);

int rtsp_ermi_getparam_parse(string str, EGETPARAM_MSG *resp_msg);

int rtsp_ermi_getparm_encode(EGETPARAM_MSG getparam, string& resp);

int rtsp_ermi_teardown_res_parse(string str,ETEARDOWN_MSG_RES*resp_msg);

int rtsp_ermi_teardown_res_encode(ETEARDOWN_MSG_RES tear,char* resp_str);

int rtsp_ermi_announce_res_encode(EANNOUNCE_MSG ann,char* resp_str);

string rtsp_ermi_response_parse(string str);

string rtsp_ermi_error_res_encode(int error_code,int cseq);

#endif
