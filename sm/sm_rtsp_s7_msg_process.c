#include "sm_rtsp_s7_msg_process.h"
#include "sm_rtsp_public_function.h"

int rtsp_s7_setup_msg_encode(S7_SETUP_MSG msg,char *setup_msg)
{
	char str[1024]="";
	char localip[40]="";
	char appip[256]="";
	char apptype[10]="";
	char ss_info[256]="";
	int i=0;

	memset(ss_info,0x00,sizeof(ss_info));
	sprintf(ss_info,"MP2T/DVBC/UDP;unicast;client=%s;destination=%s;client_port=%d",msg.ss.client,msg.ss.destination,msg.ss.client_port);
	
	memset(str,0x00,sizeof(str));
	sprintf(str,"SETUP rtsp://%s:%d RTSP/1.0\r\n"
		"CSeq:%d\r\n"
		"Require:%s\r\n"
		"OnDemandSessionId:%s\r\n"
		"SessionGroup:%s\r\n"
		"Transport:%s\r\n"
		"StreamControlProto:%s\r\n"
		"ApplicationId:%s\r\n"
		"ApplicationType:%d\r\n\r\n",
		msg.asm_ip, msg.asm_port, msg.cseq, msg.require, msg.ondemandsessionid, msg.session_group, ss_info, msg.policy, msg.app_id, msg.app_type);
	strcpy(setup_msg,str);
	return 0;
}

int rtsp_s7_setup_res_parse(char *setup_res_mag,S7_SETUP_RES *res)
{
	char str[1024];
	char *rtsp_version=NULL;
	char *line=NULL;
	char *header=NULL;
	char *header_value=NULL;
    char *subline = NULL;
	char *subheader=NULL;
	char *subheader_value=NULL;
	char *ptr=NULL;
	char *ptr1=NULL;
	char *ptr2=NULL;
	char *ptr3=NULL;
	char *ptr4=NULL;
    strcpy(str, setup_res_mag);

    line = strtok_r(str, "\r\n", &ptr);
    printf("line:%s\n", line);
    rtsp_version = strtok_r(line, " ", &ptr1);
    if(strcmp(rtsp_version, RTSP_VERSION) != 0) {
        printf("RTSP_VERSION is not matched!\n");
        return -1;
    }
    res->err_code = atoi(strtok_r(NULL, " ", &ptr1));
    if(res->err_code != RTSP_ResponseCode_OK) {
        printf("RTSP_ERR_CODE is wrong!\n");
        return -1;
    }
    line = strtok_r(NULL, "\r\n", &ptr);
    while (NULL != line) {
        if(header = strtok_r(line, ":", &ptr2)) {
            if(strcmp(header, RTSP_HEADER_CSEQ) == 0) {
                header_value = strtok_r(NULL, ":", &ptr2);
                res->cseq = atoi(header_value);
            }
            else if(strcmp(header, RTSP_HEADER_SESSION) == 0) {
                header_value = strtok_r(NULL, ":", &ptr2);
                res->session = atol(header_value);
            }
            else if(strcmp(header, RTSP_HEADER_ONDEMANDSESSIONID) == 0) {
                header_value = strtok_r(NULL, ";", &ptr2);
                strcpy(res->ondemandsessionid, header_value);
            }
            else if(strcmp(header, RTSP_HEADER_TRANSPORT) == 0) {
                header_value = strtok_r(NULL, ":", &ptr2);
                subline = strtok_r(header_value, ";", &ptr3);
                while(subline !=  NULL) {
                    if(subheader = strtok_r(subline, "=", &ptr4)) {
                        if(strcmp(subheader, "client") == 0) {
                            subheader_value = strtok_r(NULL, "=", &ptr4);
                            strcpy(res->ss.client, subheader_value);
                        }
                        else if(strcmp(subheader, "destination") == 0) {
                            subheader_value = strtok_r(NULL, "=", &ptr4);
                            strcpy(res->ss.destination, subheader_value);
                        }
                        else if(strcmp(subheader, "client_port") == 0) {
                            subheader_value = strtok_r(NULL, "=", &ptr4);
                            res->ss.client_port = atoi(subheader_value);
                        }
                        else if(strcmp(subheader, "source") == 0) {
                            subheader_value = strtok_r(NULL, "=", &ptr4);
                            strcpy(res->as.ip, subheader_value);
                        }
                        else if(strcmp(subheader, "server_port") == 0) {
                            subheader_value = strtok_r(NULL, "=", &ptr4);
                            res->as.downPort = atoi(subheader_value);
                        }
                    }
                    subline = strtok_r(NULL, ";", &ptr3);
                }
            }
        }
    line = strtok_r(NULL, "\r\n", &ptr);
    }
    return 0;
}

int rtsp_s7_teardown_msg_encode(S7_TEARDOWN_MSG msg, char *teardown_msg){
    char str[1024];
    memset(str, 0x00, 1024);
    char reason[50];
    memset(reason, 0x00, 50);
    rtsp_reason_description(msg.reason, reason);

    sprintf(str,"TEARDOWN rtsp://%s:%d RTSP/1.0\r\n"
            "CSeq:%d\r\n"
            "Require:%s\r\n"
            "Reason:%d %s\r\n"
            "Session:%llu\r\n"
            "OnDemandSessionId:%s\r\n\r\n",
            msg.asm_ip, msg.asm_port, msg.cseq, msg.require, msg.reason, reason, msg.session, msg.ondemandsessionid);
    strcpy(teardown_msg, str);
    return 0;
}

int rtsp_s7_teardown_res_parse(char *teardown_res_msg, S7_TEARDOWN_RES *res)
{
    char str[1024];
    char *line = NULL;
    char *header = NULL;
    char *header_value = NULL;
    char *ptr = NULL;
    char *ptr1 = NULL;
    char *ptr2 = NULL;
    char *ptr3 = NULL;

    strncpy(str, teardown_res_msg, strlen(teardown_res_msg));
    line = strtok_r(str, "\r\n", &ptr);
    strtok_r(line, " ", &ptr1);
    res->err_code = atoi(strtok_r(NULL, " ", &ptr1));

    line = strtok_r(NULL, "\r\n", &ptr);
    while (line != NULL)
    {
        header = strtok_r(line, ":", &ptr2);
        if(strcmp(header, RTSP_HEADER_CSEQ) == 0) {
            header_value = strtok_r(NULL, ":", &ptr2);
            res->cseq = atoi(header_value);
        }
        else if(strcmp(header, RTSP_HEADER_SESSION) == 0) {
            header_value = strtok_r(NULL, ":", &ptr2);
            res->session = atol(header_value);
        }
        else if(strcmp(header, RTSP_HEADER_ONDEMANDSESSIONID) == 0) {
            header_value = strtok_r(NULL, ":", &ptr2);
            strcpy(res->ondemandsessionid, header_value);
        }
        line = strtok_r(NULL, "\r\n", &ptr);
    }
    return 0;
}

//解析asm发给sm的announce消息
int rtsp_s7_announce_msg_parse(char *announce_msg, S7_ANNOUNCE_MSG *msg)
{
    char str[1024];
    char *rtsp_version;
    char *line = NULL;
    char *header = NULL;
	char *header_value=NULL;
    char *subline = NULL;
	char *subheader=NULL;
	char *subheader_value=NULL;
    char *type = NULL;
    char *tmp = NULL;
	char *ptr=NULL;
	char *ptr1=NULL;
	char *ptr2=NULL;
	char *ptr3=NULL;
	char *ptr4=NULL;
    char *ptr5 = NULL;
    strcpy(str, announce_msg);

    line = strtok_r(str, "\r\n", &ptr);
    type = strtok_r(line, " ", &ptr5);
    if(strcmp(type, "ANNOUNCE") != 0){
        return -1;
    }
    
    type = strtok_r(NULL, " ", &ptr5);
    tmp = strtok_r(type, "//", &ptr4);
    tmp = strtok_r(NULL, "//", &ptr4);
    header_value = strtok_r(tmp, ":", &ptr3);
    strcpy(msg->sm_ip, header_value);
    header_value = strtok_r(NULL, ":", &ptr3);
    msg->sm_port = atoi(header_value);
    rtsp_version = strtok_r(NULL, " ", &ptr5);
    if(strcmp(rtsp_version, RTSP_VERSION) != 0) {
        printf("RTSP_VERSION is not matched!\n");
        return -1;
    }
    line = strtok_r(NULL, "\r\n", &ptr);
    while (line != NULL)
    {
        if(header = strtok_r(line, ":", &ptr1)) {
            if(strcmp(header, RTSP_HEADER_CSEQ) == 0) {
                header_value = strtok_r(NULL, ":", &ptr1);
                msg->cseq = atoi(header_value);
            }
            else if(strcmp(header, RTSP_HEADER_REQUIRE) == 0) {
                header_value = strtok_r(NULL, ":", &ptr1);
                strcpy(msg->require, header_value);
            }
            else if(strcmp(header, RTSP_HEADER_SESSION) == 0) {
                header_value = strtok_r(NULL, ":", &ptr1);
                msg->session = atol(header_value);
            }
            else if(strcmp(header, RTSP_HEADER_ONDEMANDSESSIONID) == 0) {
                header_value = strtok_r(NULL, ":", &ptr1);
                strcpy(msg->ondemandsessionid, header_value);
            }
            else if(strcmp(header, RTSP_HEADER_NOTICE) == 0) {
                header_value = strtok_r(NULL, ":", &ptr1);
                subline = strtok_r(header_value, " ", &ptr2);
                msg->notice = atoi(subline);
                subline = strtok_r(NULL, " ", &ptr2);
                subheader = strtok_r(subline, ":", &ptr3);
                subheader_value = strtok_r(NULL, ":", &ptr3);
                strcpy(msg->event_date, subheader_value);
            }
        }
        line = strtok_r(NULL, "\r\n", &ptr);
    }
    return 0;
}

//创建sm发给asm的announce response消息
int rtsp_s7_announce_res_encode(S7_ANNOUNCE_RES res, char *announce_res)
{
    char str[1024];
    memset(str, 0x00, 1024);

    sprintf(str,"RTSP/1.0 %d OK\r\n"
            "CSeq:%d\r\n"
            "Session:%llu\r\n"
            "OnDemandSessionId:%s\r\n\r\n",
            res.err_code, res.cseq, res.session, res.ondemandsessionid);
    strcpy(announce_res, str);
    return 0;
}
