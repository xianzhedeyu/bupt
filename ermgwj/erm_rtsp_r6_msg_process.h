#ifndef ERM_RTSP_R6_MSG_PROCESS_H_INCLUDED
#define ERM_RTSP_R6_MSG_PROCESS_H_INCLUDED

#include <iostream>
#include <string>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
using namespace std;

typedef unsigned long long _ERM_INT64;

typedef struct _Transport_QAM
{
    string qam_destination;//目的QAM的频率和节目号
    string bandwidth;//带宽信息
    string qam_name;//QAM名字
    string client;//此属性强制，此处无意义，设为“FFFFFFFFFFFF”
} Transport_QAM;

typedef struct _Transport_UDP
{
    string destination;//目的QAM的IP
    string client_port;//目的QAM的UDP端口
    string source;//推流服务器的IP
    string server_port;//推流服务器的UDP端口
    string client;//此属性强制，此处无意义，设为“FFFFFFFFFFFF”
} Transport_UDP;

//解析RTSP URL地址，IP地址传给pPreSuffix,端口号传给pPort,QAM名传给pSuffix
bool ParseUrl(string url, string* pPreSuffix, int* pPort,string* pSuffix);
//解析response
bool GetResponses(string response,int * Code,int *CSeq,_ERM_INT64 *Session,string * OnDemandSessionId);

//创建Setup_Provision_port消息
string rtsp_r6_setup_provision_port_msg_encode(string rtspIP,int CSeq,Transport_QAM tqam,Transport_UDP tudp,int TrafficMismatch,string InbandMarker,int JitterBuffer);
//创建SetupStartCheckingRequest
string rtsp_r6_setup_start_checking_msg_encode(string rtspIP,int CSeq,_ERM_INT64 Session,Transport_QAM tqam,Transport_UDP tudp,int TrafficMismatch,string InbandMarker,int JitterBuffer);
//创建Stop Checking消息
string rtsp_r6_setup_stop_checking_msg_encode(string rtspIP,int CSeq,_ERM_INT64 Session,Transport_QAM tqam,Transport_UDP tudp,int TrafficMismatch);
//创建Teardown消息
string rtsp_r6_teardown_msg_encode(string rtspIP,Transport_QAM tqam,int CSeq,int reason,_ERM_INT64 Session,string onDemandSessionId);
//创建Ping Reaquest
string rtsp_r6_ping_msg_encode(string rtspIP,Transport_QAM tqam,int CSeq,_ERM_INT64 Session,string onDemandSessionId);
//创建Get_Parameter Reaquest
string rtsp_r6_get_parameter_msg_encode(string rtspIP,Transport_QAM tqam,int CSeq,string parameter);
//解析Get_Parameter Response
bool rtsp_r6_get_parameter_response_parse(string str);
//解析announce消息
bool rtsp_r6_announce_msg_parse(string str);

#endif // ERM_RTSP_R6_MSG_PROCESS_H_INCLUDED

