#include "ermi.h"
#include "ermi_def.h"

string ermi_trim(string s)
{
    if (s.empty()) 
    {
        return s;
    }

    s.erase(0,s.find_first_not_of(" "));
    s.erase(s.find_last_not_of(" ") + 1);
    return s;
}

int rtsp_ermi_setup_parse(string str,SETUP_MSG * resp_msg)
{
    string::size_type start,end,bflag;
    string str1;
    string str2;
    int len = 0;

    start = str.find("rtsp://");
    end = str.find("RTSP/1.0");
    resp_msg->rtsp_url = ermi_trim(str.substr(start,end-start-1));

    start = str.find("CSeq:");
    resp_msg->cseq = atoi( str.substr(start+5).c_str() );

    start = str.find("Require:");
    end = str.find("\r\n",start+1);
    resp_msg->require = ermi_trim(str.substr(start+8,end-start-8));

    start = str.find("Transport:");
    end = str.find("\r\n",start+1);
    str1 = str.substr(start+10,end-start-6);

    start = str1.find("DOCSIS/QAM");
    while(start != string::npos)
    {

        end = str1.find(",",start+1);
        if(end == string::npos)
        {
            end = str1.find("\r\n",start+1);
        }
        bflag = end;
        str2 = str1.substr(start,end-start);
        start = str2.find("unicast");
        end = str2.find(";",start+1);
        resp_msg->qaminf[len].client= ermi_trim(str2.substr(start,end-start));
        start = str2.find("bit_rate=");
        end = str2.find(";",start+1);
        resp_msg->qaminf[len].bit_rate = ermi_trim(str2.substr(start+9,end-start-9));
        start = str2.find("qam_id=");
        end = str2.find(";",start+1);
        resp_msg->qaminf[len].qam_id = ermi_trim(str2.substr(start+7,end-start-7));
        start = str2.find("depi_mode=");
        resp_msg->qaminf[len].depi_mode = str2.substr(start+10);
        start = str1.find("DOCSIS/QAM",bflag+1);
        len++;
    }
    resp_msg->qam_num = len;

    return 0;
}

int rtsp_ermi_setup_response_encode(SETUP_RESPONSE res,char *resp_str)
{
	char response[500];
	if(res.error_code == 200)
	{
		sprintf(response,"RTSP/1.0 200 OK\r\n"
			 	"CSeq: %d\r\n"
				"Session: %llu\r\n"
				"Transport:"
   				"MP2T/DVBC/UDP;unicast;client=%s;qam_destination=%s;destination=%s;client_port=%d;"
   				"qam_name=%s;qam_group=%s;modulation=%s;edge_input_group=%s\r\n",
				res.cseq,res.session,res.client.c_str(),res.qam_destination.c_str(),res.destination.c_str(),res.client_port,res.qam_name.c_str(),res.qam_group.c_str(),res.modulation.c_str(),res.edge_input_group.c_str());
	}else
	{
		string str = rtsp_ermi_error_res_encode(res.error_code,res.cseq);
		strcpy(response,str.c_str());
	}
	
	strncpy(resp_str,response,500);
	
	return 0;
}

int rtsp_ermi_teardown_parse(string str,TEARDOWN_MSG *resp_msg)
{
	string::size_type start,end;
	string rtsp_url;
	string require;
	string reason;	
	
	start = str.find("rtsp://");
   	end = str.find("RTSP/1.0");
    rtsp_url =  ermi_trim(str.substr(start+7,end-start-8));
    strcpy(resp_msg->rtsp_url,rtsp_url.c_str());
    
    start = str.find("CSeq:");
    resp_msg->cseq = atoi( str.substr(start+5).c_str() );
    
   	start = str.find("Require:");
   	end = str.find("\r\n",start+1);
    require = ermi_trim(str.substr(start+8,end-start-8));
    strcpy(resp_msg->require,require.c_str());
	
	start = str.find("Reason:");
	end = str.find("\r\n",start+1);
	reason = ermi_trim(str.substr(start+7,end-start-7));
	strcpy(resp_msg->reason,reason.c_str());
	
	start = str.find("Session:");
	resp_msg->session = atoll( str.substr(start+8).c_str() );
	
	return 0;	
}

int rtsp_ermi_teardown_encode(TEARDOWN_MSG_RES tear,char* resp_str)
{
	char msg[300];
	sprintf(msg,"TEARDOWN %s RTSP/1.0\r\n"
				"CSeq: %d\r\n"
				"Require: ermi\r\n"
				"Reason: %s\r\n"
				"Session: %llu\r\n",
				tear.rtsp_url.c_str(),tear.cseq,tear.reason.c_str(),tear.session);
	strncpy(resp_str,msg,300);
	
	return 0;
}

int rtsp_ermi_teardown_res_parse(string str,TEARDOWN_MSG_RES *resp_msg)
{
	string::size_type start,end;
	string tmp_ondemand;	
	start = str.find("CSeq:");
    resp_msg->cseq = atoi( str.substr(start+5).c_str() );
    
    start = str.find("Session:");
	resp_msg->session = atoll( str.substr(start+8).c_str() );
		
    remove(tmp_ondemand.begin(),tmp_ondemand.end(),' ');	

    return 0;	
}

int rtsp_ermi_teardown_res_encode(TEARDOWN_MSG_RES tear,char *resp_str)
{
	char res[300];
	if(tear.error_code == 200)
	{
		sprintf(res,"RTSP/1.0 200 OK\r\n"
				"CSeq: %d\r\n"
				"Session: %llu\r\n",
				tear.cseq,tear.session);
	}
	else
	{
		string str = rtsp_ermi_error_res_encode(tear.error_code,tear.cseq);
		strcpy(res,str.c_str());
	}
	strncpy(resp_str,res,300);
	return 0;
}

int rtsp_ermi_announce_res_encode(ANNOUNCE_MSG ann,char* resp_str)
{
	char announce[300];
	sprintf(announce,"ANNOUNCE %s RTSP/1.0\r\n"
				"CSeq: %d\r\n"
				"Require: ermi\r\n"
				"Session: %llu\r\n"
				"Notice: %s event-date=%s\r\n",
				ann.rtspurl.c_str(),ann.cseq,ann.session,ann.notice.c_str(),ann.event_date.c_str());
	strncpy(resp_str,announce,300);
	return 0;
}

string rtsp_ermi_response_parse(string str)
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

string rtsp_ermi_error_res_encode(int error_code,int cseq)
{
	char CSeq[256];
	string str = "RTSP/1.0 ";
	switch(error_code)
	{
		case RTSP_ERMI_ResponseCode_BadRequest:
			str += "400 Bad Request\r\n";
			break;
		case RTSP_ERMI_ResponseCode_Forbidden:
			str += "403 Forbidden\r\n";
			break;
		case RTSP_ERMI_ResponseCode_NotFound:
			str += "404 Not Found\r\n";
			break;
		case RTSP_ERMI_ResponseCode_MethodNotAllowed:
			str += "405 Method Not Allowed\r\n";
			break;
		case RTSP_ERMI_ResponseCode_NotAcceptable:
			str += "406 Not Acceptable\r\n";
			break;
		case RTSP_ERMI_ResponseCode_RequestTimeOut:
			str += "408 Request Time Out\r\n";
			break;	
		case RTSP_ERMI_ResponseCode_RequestEntityTooLarge:
			str += "413 Request Entity Too Large\r\n";
			break;	
		case RTSP_ERMI_ResponseCode_NotEnoughBandwidth:
			str += "453 Not Enough Bandwidth\r\n";
			break;		
		case RTSP_ERMI_ResponseCode_SessionNotFound:
			str += "454 Session Not Found\r\n";
			break;
		case RTSP_ERMI_ResponseCode_UnsupportedTransport:
			str += "461 Unsupported Transport\r\n";
			break;		
		case RTSP_ERMI_ResponseCode_DestinationUnreachable:
			str += "462 Destination Unreachable\r\n";
			break;	
		case RTSP_ERMI_ResponseCode_RTSPVersionNotSupported:
			str += "505 RTSP Version Not Supported\r\n";
			break;		
		case RTSP_ERMI_ResponseCode_ERMSetupFailed_InvalidRequest:
			str += "671 ERM Setup Failed �C Invalid Request\r\n";
			break;		
		case RTSP_ERMI_ResponseCode_ERMSetupFailed_QAMBandwidthNotAvailable:
			str += "672 ERM Setup Failed �C QAM Bandwidth Not Available\r\n";
			break;		
		case RTSP_ERMI_ResponseCode_ERMSetupFailed_NetworkBandwidthNotAvailable:
			str += "673 ERM Setup Failed �C Network Bandwidth Not Available\r\n";
			break;		
		case RTSP_ERMI_ResponseCode_ERMSetupFailed_ProgramNotAvailable:
			str += "674 ERM Setup Failed �C Program Not Available\r\n";
			break;		
		case RTSP_ERMI_ResponseCode_ERMSetupFailed_ServiceGroupNotFound:
			str += "675 ERM Setup Failed �C Service Group Not Found\r\n";
			break;
		case RTSP_ERMI_ResponseCode_ERMSetupFailed_QAMGroupsNotFound:
			str += "676 ERM Setup Failed �C QAM Groups Not Found\r\n";
			break;
		case RTSP_ERMI_ResponseCode_ERMSetupFailed_QAMNotAvailable:
			str += "677 ERM Setup Failed �C QAM Not Available\r\n";
			break;
		default: break;		
	}
	sprintf(CSeq, "CSeq: %u\r\n\r\n", cseq);
	str += CSeq;			
	
	return str;
}
