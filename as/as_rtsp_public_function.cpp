#include "as_rtsp_public_function.h"

//产生ondemandsessionid
int ondemandsessionid_generate(char *id)
{
	char buf[37];
	uuid_t out;
	uuid_generate(out);//产生uuid
	uuid_unparse(out,buf);//转换格式
	strncpy(id,buf,37);
	return 0;	
}
//生成NTP TIME
unsigned long NTP_time(time_t t)
{
    return (unsigned long)t+2208988800U;
}

//解析url地址
int parse_url(char *url,char *ip,int *port,char *dir)
{
    char *temp;
    char full[256] = "";
    char substr[248] = "";
    char token[248] = "";
    strncpy(full,url,256);//复制URL

    if (strncmp(full,"rtsp://",7) == 0)
    {
        strncpy(substr,full+7,248);//substr取rtsp://后字符串
        if (strchr(substr,':'))   //当url中存在端口号时
        {
            strcpy(ip,strtok(substr,":"));//取得IP
            strncpy(token,strtok(NULL,":"),248);
            *port = atoi(strtok(token,"/"));
            if (strchr(token,'/'))   //当url中存在端口号和路径时
            {
                strcpy(dir,strtok(NULL,"/"));            
                //printf("%s %d %s\n",ip,*port,dir);
            }
        }
        else if (strchr(substr,'/'))   //当url中不存在端口号但存在路径时
        {
            strcpy(ip,strtok(substr,"/"));
            strcpy(dir,strtok(NULL,"/"));
            //printf("%s %s\n",ip,dir);
        }else//当url中不存在端口号和路径时
        {
            strcpy(ip,substr);
            //printf("%s\n",ip);
        }
    }
    return 0;
}

//获取本地主机IP地址
int getlocalip(char* eth,char* localip)
{
    int sockfd;
    if (-1 == (sockfd = socket(PF_INET, SOCK_STREAM, 0)))
    {
        return -1;
    }
    char ipbuf[40];
    struct ifreq req;
    struct sockaddr_in *host;
    bzero(&req, sizeof(struct ifreq));//置字节字符串req的前sizeof(struct ifreq)个字节为零
    strcpy(req.ifr_name, eth);
    ioctl(sockfd, SIOCGIFADDR, &req);
    host = (struct sockaddr_in*)&req.ifr_addr;
    strcpy(ipbuf, (char*)inet_ntoa(host->sin_addr));//将网络地址转换为以'.'相隔的字符串形式
    close(sockfd);
    strcpy(localip,ipbuf);
    return 0;
}

//删除字符串收尾空格
char *trim(char *str)
{
    char temp[256];
    char *p = str;
    char *p1;
    if (p)
    {
        p1 = p + strlen(str) - 1;
        while (*p && isspace(*p)) p++;
        while (p1 > p && isspace(*p1)) *p1-- = '\0';
    }
    strcpy(str,p);
    return str;
}

//创建错误response消息
int rtsp_err_res_encode(int err_code,int cseq,char *err_msg)
{
    char str[300];
    char CSeq[100];
    strcpy(str,"RTSP/1.0 ");
    switch (err_code)
    {
    case RTSP_ResponseCode_BadRequest:
        strcat(str,"400 Bad Request\r\n");
        break;
    case RTSP_ResponseCode_Forbidden:
        strcat(str,"403 Forbidden\r\n");
        break;
    case RTSP_ResponseCode_NotFound:
        strcat(str,"404 Not Found\r\n");
        break;
    case RTSP_ResponseCode_MethodNotAllowed:
        strcat(str,"405 Method Not Allowed\r\n");
        break;
    case RTSP_ResponseCode_NotAcceptable:
        strcat(str,"406 Not Acceptable\r\n");
        break;
    case RTSP_ResponseCode_RequestTimeOut:
        strcat(str,"408 Request Time Out\r\n");
        break;
    case RTSP_ResponseCode_Gone:
        strcat(str,"410 Gone\r\n");
        break;
    case RTSP_ResponseCode_RequestEntityTooLarge:
        strcat(str,"413 Request Entity Too Large\r\n");
        break;
    case RTSP_ResponseCode_UnsupportedMediaType:
        strcat(str,"415 Unsupported Media Type\r\n");
        break;
    case RTSP_ResponseCode_InvalidParameter:
        strcat(str,"451 Invalid Parameter\r\n");
        break;
    case RTSP_ResponseCode_NotEnoughBandwidth:
        strcat(str,"453 Not Enough Bandwidth\r\n");
        break;
    case RTSP_ResponseCode_SessionNotFound:
        strcat(str,"454 Session Not Found\r\n");
        break;
    case RTSP_ResponseCode_InvalidRange:
        strcat(str,"457 Invalid Range\r\n");
        break;
    case RTSP_ResponseCode_AggregateOperationNotAllowed:
        strcat(str,"459 Aggregate Operation Not Allowed\r\n");
        break;
    case RTSP_ResponseCode_UnsupportedTransport:
        strcat(str,"461 Unsupported Transport\r\n");
        break;
    case RTSP_ResponseCode_DestinationUnreachable:
        strcat(str,"462 Destination Unreachable\r\n");
        break;
    case RTSP_ResponseCode_GatewayTimeout:
        strcat(str,"504 Gateway Timeout\r\n");
        break;
    case RTSP_ResponseCode_RTSPVersionNotSupported:
        strcat(str,"505 RTSP Version Not Supported\r\n");
        break;
    case RTSP_ResponseCode_ERMSetupFailed_NoResponse:
        strcat(str,"670 ERM Setup Failed - No Response\r\n");
        break;
    case RTSP_ResponseCode_ERMSetupFailed_InvalidRequest:
        strcat(str,"671 ERM Setup Failed – Invalid Request\r\n");
        break;
    case RTSP_ResponseCode_ERMSetupFailed_QAMBandwidthNotAvailable:
        strcat(str,"672 ERM Setup Failed – QAM Bandwidth Not Available\r\n");
        break;
    case RTSP_ResponseCode_ERMSetupFailed_NetworkBandwidthNotAvailable:
        strcat(str,"673 ERM Setup Failed – Network Bandwidth Not Available\r\n");
        break;
    case RTSP_ResponseCode_ERMSetupFailed_ProgramNotAvailable:
        strcat(str,"674 ERM Setup Failed – Program Not Available\r\n");
        break;
    case RTSP_ResponseCode_ERMSetupFailed_ServiceGroupNotFound:
        strcat(str,"675 ERM Setup Failed – Service Group Not Found\r\n");
        break;
    case RTSP_ResponseCode_ERMSetupFailed_QAMGroupsNotFound:
        strcat(str,"676 ERM Setup Failed – QAM Groups Not Found\r\n");
        break;
    case RTSP_ResponseCode_ERMSetupFailed_QAMNotAvailable:
        strcat(str,"677 ERM Setup Failed – QAM Not Available\r\n");
        break;
    case RTSP_ResponseCode_ERMSetupFailed_EdgeDeviceNotAvailable:
        strcat(str,"678 ERM Setup Failed - Edge Device Not Available\r\n");
        break;
    case RTSP_ResponseCode_ODRMSetupFailed_NoResponse:
        strcat(str,"750 ODRM Setup Failed - No Response\r\n");
        break;
    default:
        break;
    }
    sprintf(CSeq, "CSeq: %u\r\n\r\n", cseq);
    strcat(str,CSeq);
    strcpy(err_msg,str);

    return 0;
}

//查询Teardown消息中Reason头中编码号对应的描述
int rtsp_reason_description(int reason_code,char *description)
{
    char str[300];
    switch (reason_code)
    {
    case RTSP_TEARDOWN_REASON_CODE_User_stop:
        strcpy(str,"User stop");
        break;
    case RTSP_TEARDOWN_REASON_CODE_End_of_stream:
        strcpy(str,"End of stream");
        break;
    case RTSP_TEARDOWN_REASON_CODE_Beginning_of_stream:
        strcpy(str,"Beginning of stream");
        break;
    case RTSP_TEARDOWN_REASON_CODE_Pause_timeout:
        strcpy(str,"Pause timeout");
        break;
    case RTSP_TEARDOWN_REASON_CODE_Fail_to_tune:
        strcpy(str,"Fail to tune");
        break;
    case RTSP_TEARDOWN_REASON_CODE_loss_of_tune:
        strcpy(str,"Loss of tune");
        break;
    case RTSP_TEARDOWN_REASON_CODE_Loss_of_tune:
        strcpy(str,"Loss of tune");
        break;
    case RTSP_TEARDOWN_REASON_CODE_RTSP_failure:
        strcpy(str,"RTSP failure");
        break;
    case RTSP_TEARDOWN_REASON_CODE_Channel_failure:
        strcpy(str,"Channel failure");
        break;
    case RTSP_TEARDOWN_REASON_CODE_No_RTSP_server:
        strcpy(str,"No RTSP server");
        break;
    case RTSP_TEARDOWN_REASON_CODE_Trick_play_failed:
        strcpy(str,"Trick-play failed");
        break;
    case RTSP_TEARDOWN_REASON_CODE_Internal_ODA_issue:
        strcpy(str,"Internal ODA issue");
        break;
    case RTSP_TEARDOWN_REASON_CODE_Unknown:
        strcpy(str,"Unknown");
        break;
    case RTSP_TEARDOWN_REASON_CODE_Network_Resource_Failure:
        strcpy(str,"Network Resource Failure");
        break;
    case RTSP_TEARDOWN_REASON_CODE_Settop_Heartbeat_Timeout:
        strcpy(str,"Settop Heartbeat Timeout");
        break;
    case RTSP_TEARDOWN_REASON_CODE_Settop_Inactivity_Timeout:
        strcpy(str,"Settop Inactivity Timeout");
        break;
    case RTSP_TEARDOWN_REASON_CODE_Content_Unavailable:
        strcpy(str,"Content Unavailable");
        break;
    case RTSP_TEARDOWN_REASON_CODE_Streaming_Failure:
        strcpy(str,"Streaming Failure");
        break;
    case RTSP_TEARDOWN_REASON_CODE_QAM_Failure:
        strcpy(str,"QAM Failure");
        break;
    case RTSP_TEARDOWN_REASON_CODE_Volume_Failure:
        strcpy(str,"Volume Failure");
        break;
    case RTSP_TEARDOWN_REASON_CODE_Stream_Control_Error:
        strcpy(str,"Stream Control Error");
        break;
    case RTSP_TEARDOWN_REASON_CODE_Stream_Control_Timeout:
        strcpy(str,"Stream Control Timeout");
        break;
    case RTSP_TEARDOWN_REASON_CODE_Session_List_Mismatch:
        strcpy(str,"Session List Mismatch");
        break;
    case RTSP_TEARDOWN_REASON_CODE_Session_timeout:
        strcpy(str,"Session timeout");
        break;
    default:
        break;
    }
    strcpy(description,str);

    return 0;
}
//查询Announce消息中Notice头中编码号对应的描述
int rtsp_notice_description(int notice_code,char *description)
{
    char str[300];
    switch (notice_code)
    {
    case RTSP_ANNOUNCE_EndOfStreamReached:
        strcpy(str,"End-of-Stream Reached");
        break;
    case RTSP_ANNOUNCE_StartOfStreamReached:
        strcpy(str,"Start-of-Stream Reached");
        break;
    case RTSP_ANNOUNCE_ErrorReadingContentData:
        strcpy(str,"Error Reading Content Data");
        break;
    case RTSP_ANNOUNCE_ServerResourcesUnavailable:
        strcpy(str,"Server Resources Unavailable");
        break;
    case RTSP_ANNOUNCE_DownstreamFailure:
        strcpy(str,"Downstream Failure");
        break;
    case RTSP_ANNOUNCE_ClientSessionTerminated:
        strcpy(str,"Client Session Terminated");
        break;
    case RTSP_ANNOUNCE_InternalServerError:
        strcpy(str,"Internal Server Error");
        break;
    case RTSP_ANNOUNCE_InbandStreamMarkerMismatch:
        strcpy(str,"Inband Stream Marker Mismatch");
        break;
    case RTSP_ANNOUNCE_BandwidthExceededLimit:
        strcpy(str,"Bandwidth Exceeded Limit");
        break;
    case RTSP_ANNOUNCE_SessionInProgress:
        strcpy(str,"Session In Progress");
        break;
    case RTSP_ANNOUNCE_EncryptionEngineFailure:
        strcpy(str,"Encryption Engine Failure");
        break;
    case RTSP_ANNOUNCE_StreamBandwidthExceedsThatAvailable:
        strcpy(str,"Stream Bandwidth Exceeds That Available");
        break;
    case RTSP_ANNOUNCE_DownstreamDestinationUnreachable:
        strcpy(str,"Downstream Destination Unreachable");
        break;
    case RTSP_ANNOUNCE_UnableToEncryptOneOrMoreComponents:
        strcpy(str,"Unable to Encrypt one or more Components");
        break;
    case RTSP_ANNOUNCE_ECMGSessionFailure:
        strcpy(str,"ECMG Session Failure");
        break;
    default:
        break;
    }
    strcpy(description,str);

    return 0;
}

//获取消息类型
int rtsp_get_msg_type(char *msg)
{
	if (!strlen(msg))
        return -1;
	
	if(strstr(msg,"SETUP"))
	{
		return RTSP_ID_S1_SETUP;
	}else if(strstr(msg,"TEARDOWN"))
	{
		return RTSP_ID_S1_TEARDOWN;		
	}
	
	return -1;	
}



