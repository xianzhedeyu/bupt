#include "asm_rtsp_r8_msg_process.h"
#include "asm_rtsp_public_function.h"
int rtsp_r8_setup_msg_encode(R8_SETUP_MSG msg, char *setup_msg)
{
	char str[1024]="";
	char ss_info[256]="";

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
		msg.as_ip,msg.as_port,msg.cseq,msg.require,msg.ondemandsessionid,msg.session_group,ss_info,msg.policy,msg.app_id,msg.app_type);
	strcpy(setup_msg,str);
	return 0;
}

int rtsp_r8_setup_res_parse(char *setup_res_msg, R8_SETUP_RES *res) {
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
    strcpy(str, setup_res_msg);

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
                header_value = strtok_r(NULL, ":", &ptr2);
                strcpy(res->ondemandsessionid, header_value);
            }
            else if(strcmp(header, "Protocol") == 0) {
                header_value = strtok_r(NULL, ":", &ptr2);
                strcpy(res->protocol, header_value);
            }
            else if(strcmp(header, "As_IP") == 0) {
                header_value = strtok_r(NULL, ":", &ptr2);
                strcpy(res->as.ip, header_value);
            }
            else if(strcmp(header, "As_Port") == 0) {
                header_value = strtok_r(NULL, ":", &ptr2);
                res->as.upPort = atoi(header_value);
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
                        else if(strcmp(subheader, "server_port") == 0) {
                            subheader_value = strtok_r(NULL, "=", &ptr4);
                            res->as.downPort = atoi(subheader_value);
                        }
                        else if(strcmp(subheader, "source") == 0) {
                            subheader_value = strtok_r(NULL, "=", &ptr4);
                            strcpy(res->as.ip, subheader_value);
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

int rtsp_r8_teardown_msg_encode(R8_TEARDOWN_MSG msg, char* teardown_msg) {
    char str[1024];
    memset(str, 0x00, 1024);
    char reason[50];
    memset(reason, 0x00, 50);
    rtsp_reason_description(msg.reason, reason);

    sprintf(str, "TEARDOWN rtsp://%s:%d RTSP/1.0\r\n"
            "CSeq:%d\r\n"
            "Require:%s\r\n"
            "Reason:%d %s\r\n"
            "Session:%llu\r\n"
            "OnDemandSessionId:%s\r\n\r\n",
            msg.as_ip, msg.as_port, msg.cseq, msg.require, msg.reason, reason, msg.session, msg.ondemandsessionid);
    strcpy(str, teardown_msg);
    return 0;
}

int rtsp_r8_teardown_res_parse(char* teardown_res_msg, R8_TEARDOWN_RES* res) {
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
