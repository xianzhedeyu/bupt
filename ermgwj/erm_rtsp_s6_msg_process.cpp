#include "erm_rtsp_s6_msg_process.h"
#include "public_def.h"
//去除字符串多余空格
string trim(string s)
{
    if (s.empty())
    {
        return s;
    }

    s.erase(0,s.find_first_not_of(" "));
    s.erase(s.find_last_not_of(" ") + 1);
    return s;
}
//解析SETUP消息
int rtsp_s6_setup_msg_parse(string str,SETUP_MSG * resp_msg)
{
   // SETUP_MSG setup_msg;
    string::size_type iFind,iFindend,iFindflag;
    string str1;
    string str2;
    int len = 0;

    iFind = str.find("rtsp://");
    iFindend = str.find("RTSP/1.0");
    resp_msg->rtsp_url = trim(str.substr(iFind,iFindend-iFind-1
    ));//解析出rtsp_url

    iFind = str.find("CSeq:");
    resp_msg->cseq = atoi( str.substr(iFind+5).c_str() );//解析出cseq

    iFind = str.find("Require:");
    iFindend = str.find("\r\n",iFind+1);
    resp_msg->require = trim(str.substr(iFind+8,iFindend-iFind-8));//解析出require

    iFind = str.find("SessionGroup:");
    iFindend = str.find("\r\n",iFind+1);
    resp_msg->session_group = trim(str.substr(iFind+13,iFindend-iFind-13));//解析出session_group

    iFind = str.find("EncryptionType:");
    iFindend = str.find("\r\n",iFind+1);
    resp_msg->encryption_type = trim(str.substr(iFind+15,iFindend-iFind-15));//解析出encryption_type

    iFind = str.find("CAS_ID:");
    iFindend = str.find("\r\n",iFind+1);
    resp_msg->cas_id = trim(str.substr(iFind+7,iFindend-iFind-7));//解析出cas_id

    iFind = str.find("EncryptControl:");
    iFindend = str.find("\r\n",iFind+1);
    resp_msg->encrypt_control = trim(str.substr(iFind+15,iFindend-iFind-15));//解析出encrypt_control

    iFind = str.find("Transport:");
    iFindend = str.find("\r\n",iFind+1);
    str1 = str.substr(iFind+10,iFindend-iFind-6);//提取出Transport消息头的内容(包括"/r/n")

    iFind = str1.find("MP2T/DVBC/QAM");
    while(iFind != string::npos)
    {

        iFindend = str1.find(",",iFind+1);
        if(iFindend == string::npos)
        {
            iFindend = str1.find("\r\n",iFind+1);
        }
        iFindflag = iFindend;
        str2 = str1.substr(iFind,iFindend-iFind);//提取一个QAM的信息
        iFind = str2.find("client=");
        iFindend = str2.find(";",iFind+1);
        resp_msg->qam_info[len].client= trim(str2.substr(iFind+7,iFindend-iFind-7));//提取QAM的client信息
        iFind = str2.find("bandwidth=");
        iFindend = str2.find(";",iFind+1);
        resp_msg->qam_info[len].bandwidth = trim(str2.substr(iFind+10,iFindend-iFind-10));//提取bandwidth信息
        iFind = str2.find("qam_name=");
        iFindend = str2.find(";",iFind+1);
        resp_msg->qam_info[len].qam_name = trim(str2.substr(iFind+9,iFindend-iFind-9));//提取qam_name
        iFind = str2.find("modulation=");
        resp_msg->qam_info[len].modulation = str2.substr(iFind+11);
        iFind = str1.find("MP2T/DVBC/QAM",iFindflag+1);

        //cout << resp_msg->qam_info[len].client << endl;
        //cout << resp_msg->qam_info[len].bandwidth << endl;
        //cout << resp_msg->qam_info[len].qam_name <<endl;
        //cout <<resp_msg->qam_info[len].modulation<<endl;

        len++;
    }
    resp_msg->qam_num = len;

    iFind = str.find("OnDemandSessionId:");
    iFindend = str.find("\r\n",iFind+1);
    resp_msg->ondemandsessionid = trim(str.substr(iFind+18,iFindend-iFind-18));//提取OnDemandSessionId

    iFind = str.find("Policy:");
    iFindend = str.find("\r\n",iFind+1);
    resp_msg->policy = trim(str.substr(iFind+7,iFindend-iFind-7));//提取Policy

    iFind = str.find("InbandMarker:");
    iFindend = str.find("\r\n",iFind+1);
    resp_msg->inband_marker = trim(str.substr(iFind+13,iFindend-iFind-13));//提取InbandMarker

    //memcpy(resp_msg,&setup_msg,sizeof(setup_msg));
    //*resp_msg = setup_msg;
    return 0;
}
//创建ERM发给SM的setup response消息
int rtsp_s6_setup_response_encode(SETUP_RESPONSE res,char *resp_str)
{
	char response[500];
	if(res.error_code == 200)
	{
		sprintf(response,"RTSP/1.0 200 OK\r\n"
			 	"CSeq: %d\r\n"
				"Session: %llu\r\n"
				"Transport:"
   				"MP2T/DVBC/UDP;unicast;client=%s;qam_destination=%s;destination=%s;client_port=%d;"
   				"qam_name=%s;qam_group=%s;modulation=%s;edge_input_group=%s\r\n"
				"EmbeddedEncryptor: %s\r\n"
				"OnDemandSessionId: %s\r\n\r\n",
				res.cseq,res.session,res.client.c_str(),res.qam_destination.c_str(),res.destination.c_str(),res.client_port,res.qam_name.c_str(),res.qam_group.c_str		 (),res.modulation.c_str(),res.edge_input_group.c_str(),res.embeddedEncryptor.c_str(),res.onDemandSessionId.c_str());
	}else
	{
		string str = rtsp_s6_error_res_encode(res.error_code,res.cseq);
		strcpy(response,str.c_str());
	}

	strncpy(resp_str,response,500);

	return 0;
}
//解析SM发给ERM的teardown消息
int rtsp_s6_teardown_msg_parse(string str,TEARDOWN_MSG1 *resp_msg)
{
	//TEARDOWN_MSG1 teardown_msg;
	string::size_type iFind,iFindend;
	string rtsp_url;
	string require;
	string reason;
	string ondemandsessionid;

	iFind = str.find("rtsp://");
   	 iFindend = str.find("RTSP/1.0");
    	rtsp_url =  trim(str.substr(iFind+7,iFindend-iFind-8));//解析出rtsp_url
    	 strcpy(resp_msg->rtsp_url,rtsp_url.c_str());

    	iFind = str.find("CSeq:");
    	resp_msg->cseq = atoi( str.substr(iFind+5).c_str() );//解析出cseq

   	 iFind = str.find("Require:");
   	 iFindend = str.find("\r\n",iFind+1);
    	require = trim(str.substr(iFind+8,iFindend-iFind-8));//解析出require
    	strcpy(resp_msg->require,require.c_str());

	iFind = str.find("Reason:");
	iFindend = str.find("\r\n",iFind+1);
	reason = trim(str.substr(iFind+7,iFindend-iFind-7));//解析出reason
	strcpy(resp_msg->reason,reason.c_str());

	iFind = str.find("Session:");
	resp_msg->session = atoll( str.substr(iFind+8).c_str() );//解析出session号

	iFind = str.find("OnDemandSessionId:");
   	 iFindend = str.find("\r\n",iFind+1);
   	 ondemandsessionid = trim(str.substr(iFind+18,iFindend-iFind-18));//提取OnDemandSessionId
   	 strcpy(resp_msg->ondemandsessionid,ondemandsessionid.c_str());

	return 0;
}
//创建ERM给SM发的teardown消息
int rtsp_s6_teardown_msg_encode(TEARDOWN_MSG2 tear,char* resp_str)
{
	char msg[300];
	sprintf(msg,"TEARDOWN %s RTSP/1.0\r\n"
				"CSeq: %d\r\n"
				"Require: com.comcast.ngod.s6\r\n"
				"Reason: %s\r\n"
				"Session: %llu\r\n"
				"OnDemandSessionId: %s\r\n\r\n",
				tear.rtsp_url.c_str(),tear.cseq,tear.reason.c_str(),tear.session,tear.ondemandsessionid.c_str());
	strncpy(resp_str,msg,300);

	return 0;
}
//解析SM发给ERM的teardown response消息
int rtsp_s6_teardown_res_parse(string str,TEARDOWN_RES1 *resp_msg)
{
	string::size_type iFind,iFindend;
	string tmp_ondemand;
	iFind = str.find("CSeq:");
    resp_msg->cseq = atoi( str.substr(iFind+5).c_str() );//解析出cseq

    iFind = str.find("Session:");
	resp_msg->session = atoll( str.substr(iFind+8).c_str() );//解析出session号

	iFind = str.find("OnDemandSessionId:");
    iFindend = str.find("\r\n",iFind+1);
    tmp_ondemand = str.substr(iFind+18,iFindend-iFind-18);//提取OnDemandSessionId
    remove(tmp_ondemand.begin(),tmp_ondemand.end(),' ');//去除多余空格
	strcpy(resp_msg->ondemandsessionid,tmp_ondemand.c_str());

    return 0;
}
//创建ERM给SM发的teardown response消息
int rtsp_s6_teardown_res_encode(TEARDOWN_RES2 tear,char *resp_str)
{
	char res[300];
	if(tear.error_code == 200)
	{
		sprintf(res,"RTSP/1.0 200 OK\r\n"
				"CSeq: %d\r\n"
				"Session: %llu\r\n"
				"OnDemandSessionId: %s\r\n\r\n",
				tear.cseq,tear.session,tear.ondemandsessionid);
	}
	else
	{
		string str = rtsp_s6_error_res_encode(tear.error_code,tear.cseq);
		strcpy(res,str.c_str());
	}
	strncpy(resp_str,res,300);
	return 0;
}
//创建ERM给SM发的announce消息
int rtsp_s6_announce_res_encode(ANNOUNCE_MSG ann,char* resp_str)
{
	char announce[300];
	sprintf(announce,"ANNOUNCE %s RTSP/1.0\r\n"
				"CSeq: %d\r\n"
				"Require: com.comcast.ngod.s6\r\n"
				"Session: %llu\r\n"
				"Notice: %s event-date=%s npt=%s\r\n"
				"OnDemandSessionId: %s\r\n\r\n",
				ann.rtspurl.c_str(),ann.cseq,ann.session,ann.notice.c_str(),ann.event_date.c_str(),ann.npt.c_str(),ann.ondemandsessionid.c_str());
	strncpy(resp_str,announce,300);
	return 0;
}
//获取消息类型
string rtsp_s6__response_parse(string str)
{
	string type;
	if(!str.size())
		return "TYPE_ERROR";

	if(str.find("SETUP") != string::npos)
		type = "SETUP";
	else if (str.find("TEARDOWN") != string::npos)
		type = "TEARDOWN";
	else if (str.find("RTSP/1.0 200 OK") != string::npos)
		type = "TEARDOWN_RES";
	else if(str.find("ANNOUNCE") != string::npos)
		type = "ANNOUNCE";

	return type;
}

//创建ERM发给SM的错误response消息
string rtsp_s6_error_res_encode(int error_code,int cseq)
{
	char CSeq[256];
	string str = "RTSP/1.0 ";
	switch(error_code)
	{
		case RTSP_S6_ResponseCode_BadRequest:
			str += "400 Bad Request\r\n";
			break;
		case RTSP_S6_ResponseCode_Forbidden:
			str += "403 Forbidden\r\n";
			break;
		case RTSP_S6_ResponseCode_NotFound:
			str += "404 Not Found\r\n";
			break;
		case RTSP_S6_ResponseCode_MethodNotAllowed:
			str += "405 Method Not Allowed\r\n";
			break;
		case RTSP_S6_ResponseCode_NotAcceptable:
			str += "406 Not Acceptable\r\n";
			break;
		case RTSP_S6_ResponseCode_RequestTimeOut:
			str += "408 Request Time Out\r\n";
			break;
		case RTSP_S6_ResponseCode_Gone:
			str += "410 Gone\r\n";
			break;
		case RTSP_S6_ResponseCode_RequestEntityTooLarge:
			str += "413 Request Entity Too Large\r\n";
			break;
		case RTSP_S6_ResponseCode_UnsupportedMediaType:
			str += "415 Unsupported Media Type\r\n";
			break;
		case RTSP_S6_ResponseCode_InvalidParameter:
			str += "451 Invalid Parameter\r\n";
			break;
		case RTSP_S6_ResponseCode_NotEnoughBandwidth:
			str += "453 Not Enough Bandwidth\r\n";
			break;
		case RTSP_S6_ResponseCode_SessionNotFound:
			str += "454 Session Not Found\r\n";
			break;
		case RTSP_S6_ResponseCode_InvalidRange:
			str += "457 Invalid Range\r\n";
			break;
		case RTSP_S6_ResponseCode_AggregateOperationNotAllowed:
			str += "459 Aggregate Operation Not Allowed\r\n";
			break;
		case RTSP_S6_ResponseCode_UnsupportedTransport:
			str += "461 Unsupported Transport\r\n";
			break;
		case RTSP_S6_ResponseCode_DestinationUnreachable:
			str += "462 Destination Unreachable\r\n";
			break;
		case RTSP_S6_ResponseCode_GatewayTimeout:
			str += "504 Gateway Timeout\r\n";
			break;
		case RTSP_S6_ResponseCode_RTSPVersionNotSupported:
			str += "505 RTSP Version Not Supported\r\n";
			break;
		case RTSP_S6_ResponseCode_ERMSetupFailed_InvalidRequest:
			str += "671 ERM Setup Failed – Invalid Request\r\n";
			break;
		case RTSP_S6_ResponseCode_ERMSetupFailed_QAMBandwidthNotAvailable:
			str += "672 ERM Setup Failed – QAM Bandwidth Not Available\r\n";
			break;
		case RTSP_S6_ResponseCode_ERMSetupFailed_NetworkBandwidthNotAvailable:
			str += "673 ERM Setup Failed – Network Bandwidth Not Available\r\n";
			break;
		case RTSP_S6_ResponseCode_ERMSetupFailed_ProgramNotAvailable:
			str += "674 ERM Setup Failed – Program Not Available\r\n";
			break;
		case RTSP_S6_ResponseCode_ERMSetupFailed_ServiceGroupNotFound:
			str += "675 ERM Setup Failed – Service Group Not Found\r\n";
			break;
		case RTSP_S6_ResponseCode_ERMSetupFailed_QAMGroupsNotFound:
			str += "676 ERM Setup Failed – QAM Groups Not Found\r\n";
			break;
		case RTSP_S6_ResponseCode_ERMSetupFailed_QAMNotAvailable:
			str += "677 ERM Setup Failed – QAM Not Available\r\n";
			break;
		case RTSP_S6_ResponseCode_ERMSetupFailed_EdgeDeviceNotAvailable:
			str += "678 ERM Setup Failed - Edge Device Not Available\r\n";
			break;
		default: break;
	}
	sprintf(CSeq, "CSeq: %u\r\n\r\n", cseq);
	str += CSeq;

	return str;
}
