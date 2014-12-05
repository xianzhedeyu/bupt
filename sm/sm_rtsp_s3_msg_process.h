#ifndef SM_RTSP_S3_H
#define SM_RTSP_S3_H

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

//SM发给ODRM的SETUP消息中QAM的相关信息
typedef struct _S3_QAM
{
    char client[128];
    char destination[40];
    int client_port;
    INT64 bandwidth;
}S3_QAM;
//创建SM发给ODRM的SETUP信息
typedef struct _S3_SETUP_MSG
{
	char odrm_ip[40];
	int odrm_port;
	int cseq;
	char require[40];
	char ondemand_session_id[256];
	char sop_group[5][20];
	S3_QAM qam[QAM_NUM_MAX];
	int qam_num;
	char session_group[20];
	int start_point_slot;
	char start_point_npt[50];
	char policy[128];
	char inband_marker[256];
	char content_type[30];
	//sdp消息参数
	int sdp_version;		//NGOD中默认为"0"
	char email_add[5];		//NGOD中默认为"-"
	char ntp[50];			//the time that the session setup message was created
	char add_type[10];		//"IN"
	char ip_version[10];	//"IP4"
	char sm_ip[40];			//创建会话的服务器地址，SM或ODRM的IP，此处应为SM的IP
	char s[2];				//NGOD中默认为" "
	int time[2];			//Describes the validity start/end times of the session. 0 indicates media is always available
	//a=X-playlist-item
	char provider_id[128];	//the CableLabs provider_id for the content asset	
	char asset_id[40];		//the CableLabs asset_id for the content asset	
	int start_npt;			// an 8-digit hex ASCII value, no “0x” prefix and no leading zero
	int stop_npt;			// an 8-digit hex ASCII value, no “0x” prefix and no leading zero			
	
	char c[15];				//NGOD中默认为"IN IP4 0.0.0.0 "
	char m[17];				//NGOD中默认为"video 0 udp MP2T"					
}S3_SETUP_MSG;

//ODRM发给SM的SETUP RESPONSE消息中用到的参数
typedef struct _S3_SETUP_RES
{
	int err_code;
	int cseq;
	INT64 session;
	char ondemand_session_id[128];
	char sop_group[50];
	char sop[50];
	
    char client[128];
    char destination[40];
    int client_port;
    char source[40];
    int server_port;    
    INT64 bandwidth;
	
	char content_type[50];
	int content_length;
	
	//sdp消息参数
	int sdp_version;		//NGOD中默认为"0"
	char email_add[5];		//NGOD中默认为"-"
	INT64 ss_session;		//RTSP session ID of the Streaming Server,S1接口中使用此参数来发送控制消息C1
	char ntp[50];			//the time that the session setup message was created
	char add_type[10];		//"IN"
	char ip_version[10];	//"IP4"
	char ss_ip[40];			//IP address of the RTSP server on the Streaming Server
	char s[2];				//NGOD中默认为" "
	int time[2];			//Describes the validity start/end times of the session. 0 indicates media is always available
	char protocol[10];		//例如rtsp, lscp, lscpu
	char host[40];			//the IP address or fully qualified DNS name of the stream control port
	int port;				//the TCP or UDP port number of the stream control port.
	INT64 stream_handle;	//the RTSP session ID or LSCP stream handle to control this stream, depending on the <protocol> value.
	char c[15];				//NGOD中默认为"IN IP4 0.0.0.0 "
	char m[17];				//NGOD中默认为"video 0 udp MP2T"			
}S3_SETUP_RES;

//SM给ODRM发的teardown消息
typedef struct _S3_TEARDOWN_MSG
{
	char odrm_ip[40];
	int odrm_port;
	int cseq;
	char require[128];
	int reason;
	INT64 session;
	char ondemand_session_id[128];
}S3_TEARDOWN_MSG;

//ODRM发给SM的teardown response
typedef struct _S3_TEARDOWN_RES
{
	int err_code;
	int cseq;
	INT64 session;
	char ondemand_session_id[128];
	int stop_point_slot;
	char stop_point_npt[50];
}S3_TEARDOWN_RES;

//ODRM给SM发的announce消息
typedef struct _S3_ANNOUNCE_MSG
{
	char sm_ip[40];
	int sm_port;
	int cseq;
	char require[128];
	INT64 session;
	int notice;
	char event_date[30];
	char npt[30];
	char ondemand_session_id[128];
}S3_ANNOUNCE_MSG;

//SM给ODRM发的ANNOUNCE RESPONSE
typedef struct _S3_ANNOUNCE_RES
{
	int err_code;
	int cseq;
	INT64 session;
	char ondemand_session_id[128];
}S3_ANNOUNCE_RES;

//SM给ODRM发的GET_PARAMETER消息
typedef struct _S3_GET_PARAMETER_MSG
{
	char odrm_ip[40];
	int odrm_port;
	int cseq;
	INT64 session;
	char require[128];
	char content_type[50];
	int content_length;
	char parameter[50];
}S3_GET_PARAMETER_MSG;
//ODRM发给SM的GET_PARAMETER RESPONSE
typedef struct _S3_GET_PARAMETER_RES
{
	int err_code;
	int cseq;
	INT64 session;
	char content_type[50];
	char content_length;
	char parameter_val[300];
}S3_GET_PARAMETER_RES;

//SM给ODRM发的SET_PARAMETER消息
typedef struct _S3_SET_PARAMETER_MSG
{
	char odrm_ip[40];
	int odrm_port;
	int cseq;
	char require[128];
	char content_type[50];
	int content_length;
	char content[50];
}S3_SET_PARAMETER_MSG;

//ODRM发给SM的SET_PARAMETER RESPONSE
typedef struct _S3_SET_PARAMETER_RES
{
	int err_code;
	int cseq;
}S3_SET_PARAMETER_RES;



//创建SM发给ODRM的SETUP信息
int rtsp_s3_setup_msg_encode(S3_SETUP_MSG msg,char *setup_msg);
//创建ODRM发给SM的SETUP RESPONSE消息
int rtsp_s3_setup_res_parse(char *setup_res,S3_SETUP_RES *res);
//创建SM发给ODRM的TEARDOWN消息
int rtsp_s3_teardown_msg_encode(S3_TEARDOWN_MSG tear,char *tear_msg);
//解析ODRM发给SM的TEARDDOWN RESPONSE消息
int rtsp_s3_teardown_res_parse(char *tear_res,S3_TEARDOWN_RES *res);
//解析ERM向SM发送的ANNOUNCE消息
int rtsp_s3_announce_msg_parse(char * announce,S3_ANNOUNCE_MSG *ann);
//创建SM向ERM发送的ANNOUNCE RESPONSE消息
int rtsp_s3_announce_res_encode(S3_ANNOUNCE_RES res,char *ann_res);
//创建SM向ODRM发送的GET PARAMETER消息
int rtsp_s3_get_parameter_msg_encode(S3_GET_PARAMETER_MSG msg,char *get_parameter);
//解析ODRM发给SM的GET_PARAMETER RESPONSE消息
int rtsp_s3_get_parameter_res_parse(char *get_parameter_res,S3_GET_PARAMETER_RES *res);
//创建SM向ODRM发送的SET PARAMETER消息
int rtsp_s3_set_parameter_msg_encode(S3_SET_PARAMETER_MSG msg,char *set_parameter);
//解析ODRM发给SM的SET_PARAMETER RESPONSE消息
int rtsp_s3_set_parameter_res_parse(char *set_parameter_res,S3_SET_PARAMETER_RES *res);


#endif



