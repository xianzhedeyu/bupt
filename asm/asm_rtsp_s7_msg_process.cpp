#include "asm_rtsp_s7_msg_process.h"
#include "asm_rtsp_public_function.h"

int rtsp_s7_setup_res_encode(S7_SETUP_RES resmsg, char *setup_res_msg)
{
	char str[1024]="";
	
	memset(str,0x00,sizeof(str));
	if(resmsg.err_code==200){
		sprintf(str,"RTSP/1.0 %d OK\r\n"
			"CSeq:%d\r\n"
			"Session:%llu\r\n"
            "protocol:%s\r\n"
			"OnDemandSessionId:%s\r\n"
            "streamhandle:%llu\r\n"
			"Transport:"
			"MP2T/DVBC/UDP;unicast;client=%s;destination=%s;client_port=%d;source=%s;server_port=%d\r\n",
			resmsg.err_code,resmsg.cseq,resmsg.session,resmsg.protocol,resmsg.ondemandsessionid,resmsg.ondemandsessionid,resmsg.ss.client, resmsg.ss.destination, resmsg.ss.client_port, resmsg.as.ip, resmsg.as.downPort);			
	}
	strcpy(setup_res_msg,str);
	return 0;
}
int rtsp_s7_setup_msg_parse(char *setup_msg, S7_SETUP_MSG *msg)
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
    strcpy(str, setup_msg);

    line = strtok_r(str, "\r\n", &ptr);
    type = strtok_r(line, " ", &ptr5);
    if(strcmp(type, "SETUP") != 0){
        return -1;
    }
    type = strtok_r(NULL, " ", &ptr5);
    tmp = strtok_r(type, "//", &ptr4);
    tmp = strtok_r(NULL, "//", &ptr4);
    header_value = strtok_r(tmp, ":", &ptr3);
    strcpy(msg->asm_ip, header_value);
    header_value = strtok_r(NULL, ":", &ptr3);
    msg->asm_port = atoi(header_value);
    rtsp_version = strtok_r(NULL, " ", &ptr5);
    if(strcmp(rtsp_version, RTSP_VERSION) != 0) {
        printf("RTSP_VERSION is not matched!\n");
        return -1;
    }
    line = strtok_r(NULL, "\r\n", &ptr);
    while (NULL != line) {
        if(header = strtok_r(line, ":", &ptr1)) {
            if(strcmp(header, RTSP_HEADER_CSEQ) == 0) {
                header_value = strtok_r(NULL, ":", &ptr1);
                msg->cseq = atoi(header_value);
            }
            else if(strcmp(header, RTSP_HEADER_REQUIRE) == 0) {
                header_value = strtok_r(NULL, ":", &ptr1);
                strcpy(msg->require, header_value);
            }
            else if(strcmp(header, RTSP_HEADER_ONDEMANDSESSIONID) == 0) {
                header_value = strtok_r(NULL, ":", &ptr1);
                strcpy(msg->ondemandsessionid, header_value);
            }
            else if(strcmp(header, RTSP_HEADER_SESSION_GUOUP) == 0) {
                header_value = strtok_r(NULL, ":", &ptr1);
                strcpy(msg->session_group, header_value);
            }
            else if(strcmp(header, RTSP_HEADER_POLICY) == 0) {
                header_value = strtok_r(NULL, ":", &ptr1);
                strcpy(msg->policy, header_value);
            }
            else if(strcmp(header, "ApplicationId") == 0) {
                header_value = strtok_r(NULL, ":", &ptr1);
                strcpy(msg->app_id, header_value);
            }
            else if(strcmp(header, "ApplicationType") == 0) {
                header_value = strtok_r(NULL, ":", &ptr1);
                msg->app_type = atoi(header_value);
            }
            else if(strcmp(header, RTSP_HEADER_TRANSPORT) == 0) {
                header_value = strtok_r(NULL, ":", &ptr1);
                subline = strtok_r(header_value, ";", &ptr2);
                while(subline != NULL) {
                    if(subheader = strtok_r(subline, "=", &ptr3)) {
                        if(strcmp(subheader, "client") == 0) {
                            subheader_value = strtok_r(NULL, "=", &ptr3);
                            strcpy(msg->ss.client, subheader_value);
                        }
                        else if(strcmp(subheader, "destination") == 0) {
                            subheader_value = strtok_r(NULL, "=", &ptr3);
                            strcpy(msg->ss.destination, subheader_value);
                        }
                        else if(strcmp(subheader, "client_port") == 0) {
                            subheader_value = strtok_r(NULL, "=", &ptr3);
                            msg->ss.client_port = atoi(subheader_value);
                        }
                    }
                    subline = strtok_r(NULL, ";", &ptr2);
                }
            }
        }
        line = strtok_r(NULL, "\r\n", &ptr);
    }
    return 0;
}

//创建asm向sm发送的announce消息
int rtsp_s7_announce_msg_encode(S7_ANNOUNCE_MSG msg, char *announce_msg)
{
    char str[1024];
    memset(str, 0x00, 1024);
    
    sprintf(str, "ANNOUNCE rtsp://%s:%d RTSP/1.0\r\n"
            "CSeq:%d\r\n"
            "Require:%s\r\n"
            "Session:%llu\r\n"
            "Notice:%d event_date=%s npt=\r\n"
            "OnDemandSessionId:%s\r\n",
            msg.sm_ip, msg.sm_port, msg.cseq, msg.require, msg.session, msg.notice, msg.event_date, msg.ondemandsessionid);
    strcpy(announce_msg, str);
    return 0;
}
//解析sm向asm发送的announce response消息
int rtsp_s7_announce_res_parse(char *res_msg, S7_ANNOUNCE_RES *res)
{
    char str[1024];
    char *line = NULL;
    char *header = NULL;
    char *header_value = NULL;
    char *ptr = NULL;
    char *ptr1 = NULL;
    char *ptr2 = NULL;
    char *ptr3 = NULL;

    strncpy(str, res_msg, strlen(res_msg));
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
//创建asm向sm发送的teardown response消息
int rtsp_s7_teardown_res_encode(S7_TEARDOWN_RES resmsg, char* res)
{
    char str[1024];
    memset(str, 0x00, 1024);
    if(resmsg.err_code == 200)
    {
        sprintf(str, "RTSP/1.0 200 OK\r\n"
                "CSeq:%d\r\n"
                "Session:%llu\r\n"
                "OnDemandSessionId:%s\r\n\r\n",
                resmsg.cseq, resmsg.session, resmsg.ondemandsessionid);
        strcpy(res, str);
    }
    else
    {
    }
    return 0;
}
//解析sm向asm发送的teardown消息
int rtsp_s7_teardown_msg_parse(char* teardown_msg, S7_TEARDOWN_MSG *msg)
{
    char str[1024];
    char* line = NULL;
    char* type = NULL;
    char* header = NULL;
    char* header_value = NULL;
    char* tmp = NULL;
    char* ptr = NULL;
    char* ptr1 = NULL;
    char* ptr2 = NULL;
    char* ptr3 = NULL;
    char* ptr4 = NULL;
    char* ptr5 = NULL;

    strncpy(str, teardown_msg, strlen(teardown_msg));
    line = strtok_r(str, "\r\n", &ptr);
    type = strtok_r(line, " ", &ptr1);
    if(strcmp(type, "TEARDOWN") != 0) {
        return -1;
    }
    type = strtok_r(NULL, " ", &ptr1);
    strtok_r(type, "//", &ptr2);
    tmp = strtok_r(NULL, "//", &ptr2);
    strcpy(msg->asm_ip, strtok_r(tmp, ":", &ptr3));
    msg->asm_port = atoi(strtok_r(NULL, ":", &ptr3));
    header_value = strtok_r(NULL, " ", &ptr1);
    if(strcmp(header_value, RTSP_VERSION) != 0) {
        printf("RTSP_VERSION is not matched!\n");
        return -1;
    }
    line = strtok_r(NULL, "\r\n", &ptr);
    while(line != NULL) {
        header = strtok_r(line, ":", &ptr4);
        header_value = strtok_r(NULL, ":", &ptr4);
        if(strcmp(header, RTSP_HEADER_CSEQ) == 0) {
            msg->cseq = atoi(header_value);
        }
        else if(strcmp(header, RTSP_HEADER_REQUIRE) == 0) {
            strcpy(msg->require, header_value);
        }
        else if(strcmp(header, RTSP_HEADER_REASON) == 0) {
            msg->reason = atoi(strtok_r(header_value, " ", &ptr5));
        }
        else if(strcmp(header, RTSP_HEADER_SESSION) == 0) {
            msg->session =atol(header_value);
        }
        else if(strcmp(header, RTSP_HEADER_ONDEMANDSESSIONID) == 0) {
            strcpy(msg->ondemandsessionid, header_value);
        }
        line = strtok_r(NULL, "\r\n", &ptr);
    }
    return 0;
}
