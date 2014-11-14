#include "erm_rtsp_r6_msg_process.h"
#include"public_def.h"

//解析RTSP URL地址，IP地址传给pPreSuffix,端口号传给pPort,QAM名传给pSuffix
bool ParseUrl(string url, string* pPreSuffix, int* pPort,string* pSuffix)
{
	int port;
	string preSuffix;
	string suffix;
	string::size_type iFind, iFindEnd;
	iFind = url.find("rtsp://");
	if (iFind == string::npos)
	{
		printf("R6:rtsp: bad url: %s\n", url.c_str());
		return false;
	}
	url.erase(0, iFind + 7);	//remove "rtsp://"

	iFind = url.find(':');
	if (iFind != string::npos)
	{
		iFindEnd = url.find('/', iFind);
		if (iFindEnd != string::npos)
		{
			port = atoi( url.substr(iFind + 1, iFindEnd - iFind-1).c_str() );//port端口号
			url.erase(iFind, iFindEnd - iFind);
		}
	}

	iFind = url.find('/');
	if (iFind != string::npos)
	{
		preSuffix = url.substr(0, iFind - 0);//IP地址
		url.erase(0, iFind + 1);
		suffix = url;//绝对地址的路径
	}
	else
	{
		preSuffix = url;
	}
	if (pPreSuffix)
		*pPreSuffix = preSuffix;
	if (pSuffix)
		*pSuffix = suffix;
	if (pPort)
		*pPort = port;

	return true;
}

//解析response
bool GetResponses(string response,int * Code,int *CSeq,_ERM_INT64 *Session,string * OnDemandSessionId)
{
	string str =  response;
	string::size_type iFind,iFindend;
	int code;
	int cseq;
	_ERM_INT64 session;
	string ondemand_session_id;
	//str = "RTSP/1.0 200 OK\r\nCSeq:316\r\nSession:47112344\r\nOnDemandSessionId: be074250-cc5a-11d9-8cd5-0800200c9a66\r\n";
	if ( !str.size() )
	{
		*Code=RTSP_S6_ResponseCode_BadRequest;
		return false;//如果str为空，返回false
	}

	iFind = str.find("RTSP/1.0");
	code = atoi( str.substr(iFind+8).c_str() );//解析出response codes
	*Code = code;

	iFind = str.find("CSeq:");
	if ( iFind != string::npos )
	{
		cseq = atoi( str.substr(iFind+5).c_str() );//解析出cseq
		*CSeq = cseq;
	}

	iFind = str.find("Session:");
	if ( iFind != string::npos )
	{
		session = atoll( str.substr(iFind+8).c_str() );//解析出session号
		*Session = session;
	}

	iFind = str.find("OnDemandSessionId:");
	if(iFind != -1)
	{
		iFindend = str.find("\r\n",iFind+1);
		ondemand_session_id = str.substr(iFind+18,iFindend-iFind-18);//解析出的ondemand_session_id
		remove(ondemand_session_id.begin(),ondemand_session_id.end(),' ');
		* OnDemandSessionId = ondemand_session_id;
	}
	if(code!=200) 
		return false;
	return true;
}
//创建通用request消息头,包含request type和cseq
string GenerateCommonRequest(string requestType,string rtspUrl,int CSeq)
{
	string requestCmd;
	char cseq[256];
	requestCmd = requestType;//添加消息类型
	requestCmd += " ";
	requestCmd += rtspUrl;//添加RTSP URL
	requestCmd += " RTSP/1.0\r\n";//添加RTSP版本号
	sprintf(cseq, "CSeq: %u", CSeq);
	requestCmd += cseq;//添加CSeq
	requestCmd += "\r\n";
	return requestCmd;
}
//添加session消息头
string AddSessionID(_ERM_INT64 Session)
{
	string str;
	char session[256];
	sprintf(session, "Session: %llu", Session);
	str = session;//添加session号
	str += "\r\n";
	return str;
}
//添加provisionport消息头
string AddProvisionPortHeader()
{
	string str = "ProvisionPort: 1\r\n";//添加ProvisionPort消息头
	return str;
}
//添加startchecking消息头
string AddStartCheckingHeader()
{
	string str = "StartChecking: 1\r\n";//添加StartChecking消息头
	return str;
}
//添加stopchecking消息头
string AddStopCheckingHeader()
{
	string str = "StopChecking: 1\r\n";//添加StopChecking消息头
	return str;
}
//添加require消息头
string AddRequireHeader()
{
	string str = "Require: com.comcast.ngod.r6\r\n";//添加Require消息头
	return str;
}
//添加transport（包含QAM，UDP）消息头
string AddTransportHeader_QAM_UDP(Transport_QAM tqam,Transport_UDP tudp)
{
	string str;
	str = "Transport: ";
	//添加QAM参数
	str += "MP2T/DVBC/QAM;unicast;";
	str += "qam_destination=";
	str += tqam.qam_destination;//添加qam_destination(包括频率和节目号)
	str += ";bandwidth=";
	str += tqam.bandwidth;//添加所需的带宽
	str += ";qam_name=";
	str += tqam.qam_name;//添加qam name
	str += ";client=";
	str += tqam.client;//添加client（此属性强制，此处无意义，设为“FFFFFFFFFFFF”）

	str += ",";
	//添加UDP参数
	str += "MP2T/DVBC/UDP;unicast;";
	str += "destination=";
	str += tudp.destination;//添加访问QAM的IP地址
	str += ";client_port=";
	str +=  tudp.client_port;//添加访问QAM的UDP端口
	str += ";source=";
	str += tudp.source;//添加推流服务器的IP地址
	str += ";server_port=";
	str += tudp.server_port;//添加推流服务器的UDP端口
	str += ";client=";
	str += tudp.client;//添加client（此属性强制，此处无意义，设为“FFFFFFFFFFFF”）
	str += "\r\n";

	return str;
}
//添加transport（仅包含UDP）消息头
string AddTransportHeader_UDP(Transport_UDP tudp)
{
	string str;
	str = "Transport: ";
	//添加UDP参数
	str += "MP2T/DVBC/UDP;unicast;";
	str += "destination=";
	str += tudp.destination;//添加访问QAM的IP地址
	str += ";client_port=";
	str +=  tudp.client_port;//添加访问QAM的UDP端口
	str += ";source=";
	str += tudp.source;//添加推流服务器的IP地址
	str += ";server_port=";
	str += tudp.server_port;//添加推流服务器的UDP端口
	str += ";client=";
	str += tudp.client;//添加client（此属性强制，此处无意义，设为“FFFFFFFFFFFF”）
	str += "\r\n";

	return str;
}
//添加ReportTrafficMismatch消息头，参数范围0到3
string AddReportTrafficMismatchHeader(int r)
{
	string str = "ReportTrafficMismatch: ";
	char a[2];
	sprintf(a,"%d",r);
	str += a;
	str += "\r\n";
	return str;
}
//添加InbandMarker消息头
string AddInbandMarkerHeader(string InbandMarker)
{
	string str = "InbandMarker:" ;
	str += InbandMarker;
	str += "\r\n";
	return str;
}
//添加JitterBuffer消息头
string AddJitterBufferHeader(int j)
{
	string str = "JitterBuffer: ";
	char a[20];
	sprintf(a,"%d",j);
	str += a;
	str += "\r\n";
	return str;
}
//添加Reason消息头
string AddReasonHeader()
{
	string str = "Reason: ";
	str += "200 \"User stop\"";
	str += "\r\n";

	return str;
}
//添加OnDemandSessionId消息头
string AddOnDemandSessionIdHeader(string onDemandSessionId)
{
	string str = "OnDemandSessionId: ";
	str += onDemandSessionId;
	str += "\r\n";
	return str;
}
//添加Get_Parameter消息需要的参数
string AddParameter(string parameter)
{
	char a[20];
	string str = "Content-Type: text/parameters\r\n";
	str += "Content-Length: ";
	sprintf(a,"%d",parameter.length());//获取参数的长度
	str += a;//添加参数的长度
	str += "\r\n\r\n";
	str += parameter;//添加参数的内容

	return str;
}
//创建Setup_Provision_port消息
string rtsp_r6_setup_provision_port_msg_encode(string rtspIP,int CSeq,Transport_QAM tqam,Transport_UDP tudp,int TrafficMismatch,string InbandMarker,int JitterBuffer)
{
	string requestcmd;
	string rtspUrl = "rtsp://" + rtspIP + "/" + tqam.qam_name;
	requestcmd = GenerateCommonRequest("SETUP",rtspUrl,CSeq);//添加通用消息头
	requestcmd += AddProvisionPortHeader();//添加ProvisionPort消息头
	requestcmd += AddRequireHeader();//添加Require消息头
	requestcmd += AddTransportHeader_QAM_UDP(tqam,tudp);//添加Transport消息头（包括qam和udp信息）
	requestcmd += AddReportTrafficMismatchHeader(TrafficMismatch);//添加TrafficMismatch消息头
	if(TrafficMismatch == 1)
	{
		requestcmd += AddInbandMarkerHeader(InbandMarker);
	}
	requestcmd +=	AddJitterBufferHeader(JitterBuffer);
	//requestcmd += "OnDemandSessionId: ";//be074250-cc5a-11d9-8cd5-0800200c9a66";
	requestcmd += "\r\n";
	return requestcmd;
}
//创建SetupStartCheckingRequest
string rtsp_r6_setup_start_checking_msg_encode(string rtspIP,int CSeq,_ERM_INT64 Session,Transport_QAM tqam,Transport_UDP tudp,int TrafficMismatch,string InbandMarker,int JitterBuffer)
{
	string requestcmd;
	string rtspUrl = "rtsp://" + rtspIP + "/" + tqam.qam_name;
	requestcmd = GenerateCommonRequest("SETUP",rtspUrl,CSeq);//添加通用消息头
	requestcmd += AddSessionID(Session);//添加session号
	requestcmd += AddStartCheckingHeader();//添加StartChecking消息头
	requestcmd += AddRequireHeader();//添加Require消息头
	requestcmd += AddTransportHeader_UDP(tudp);//添加Transport消息头
	requestcmd += AddReportTrafficMismatchHeader(TrafficMismatch);//添加ReportTrafficMismatch消息头
	requestcmd += AddInbandMarkerHeader(InbandMarker);//添加InbandMarker消息头
	requestcmd += AddJitterBufferHeader(JitterBuffer);//添加JitterBuffer消息头
	// requestcmd += "OnDemandSessionId:";
	requestcmd += "\r\n";

	return requestcmd;
}
//创建Stop Checking消息
string rtsp_r6_setup_stop_checking_msg_encode(string rtspIP,int CSeq,_ERM_INT64 Session,Transport_QAM tqam,Transport_UDP tudp,int TrafficMismatch)
{
	string requestcmd;
	string rtspUrl = "rtsp://" + rtspIP + "/" + tqam.qam_name;
	requestcmd = GenerateCommonRequest("SETUP",rtspUrl,CSeq);//添加通用消息头
	requestcmd += AddSessionID(Session);//添加session号
	requestcmd += AddStopCheckingHeader();//添加StopChecking消息头
	requestcmd += AddRequireHeader();//添加Require消息头
	requestcmd += AddTransportHeader_UDP(tudp);//添加Transport消息头（仅含UDP信息，添加此消息头的目的是检查推流服务器的IP和端口号）
	requestcmd += AddReportTrafficMismatchHeader(TrafficMismatch);//添加ReportTrafficMismatch消息头
	requestcmd += "\r\n";

	return requestcmd;
}
//创建Teardown消息
string rtsp_r6_teardown_msg_encode(string rtspIP,Transport_QAM tqam,int CSeq,int reason,_ERM_INT64 Session,string onDemandSessionId)
{
	string requestcmd;
	string rtspUrl = "rtsp://" + rtspIP + "/" + tqam.qam_name;
	requestcmd = GenerateCommonRequest("TEARDOWN",rtspUrl,CSeq);//添加通用消息头
	requestcmd += AddRequireHeader();//添加Require消息头
	requestcmd += AddReasonHeader();//添加Reason消息头
	requestcmd += AddSessionID(Session);//添加session号
	requestcmd += AddOnDemandSessionIdHeader(onDemandSessionId);//添加OnDemandSessionId
	requestcmd += "\r\n";
	return requestcmd;
}
//创建Ping Reaquest
string rtsp_r6_ping_msg_encode(string rtspIP,Transport_QAM tqam,int CSeq,_ERM_INT64 Session,string onDemandSessionId)
{
	string requestcmd;
	string rtspUrl = "rtsp://" + rtspIP + "/" + tqam.qam_name;
	requestcmd = GenerateCommonRequest("PING",rtspUrl,CSeq);//添加通用消息头
	requestcmd += AddRequireHeader();//添加Require消息头
	requestcmd += AddSessionID(Session);//添加session号
	requestcmd += AddOnDemandSessionIdHeader(onDemandSessionId);//添加OnDemandSessionId
	requestcmd += "\r\n";
	return requestcmd;
}
//创建Get_Parameter Request
string rtsp_r6_get_parameter_msg_encode(string rtspIP,Transport_QAM tqam,int CSeq,string parameter)
{
	string requestcmd;
	string rtspUrl = "rtsp://" + rtspIP + "/" + tqam.qam_name;
	requestcmd = GenerateCommonRequest("GET_PARAMETER",rtspUrl,CSeq);//添加通用消息头
	requestcmd += AddRequireHeader();//添加Require消息头
	requestcmd += AddParameter(parameter);//添加要获取的参数
	requestcmd += "\r\n";
	return requestcmd;
}
//解析Get_Parameter Response
bool rtsp_r6_get_parameter_response_parse(string str)
{
	//string str = "RTSP/1.0 200 OK\r\nCseq: 316\r\nContent_type: text/parameters\r\nContent_Length: 23\r\n\r\nconnection_timeout: 180";
	//string str = "RTSP/1.0 200 OK\r\nCseq: 316\r\nContent_type: text/parameters\r\nContent_Length: 23\r\n\r\nsession_list:12345:b50557b0-fecc-11d9-8cd6-0800200c9a66
	//12346:dec1b300-fecc-11d9-8cd6-0800200c9a66"; 
	string parameter;
	int connection_timeout;
	string session_list;
	string::size_type iFind,iFindend;

	if(str.find("RTSP/1.0 200 OK") != -1)
	{
		iFind = str.find("connection_timeout:");//如果要获取的参数是connection_timeout
		if(iFind != -1)
		{
			connection_timeout = atoi( str.substr(iFind+19).c_str() );//获取connection timeout的值
			printf("R6:%d\n",connection_timeout);
		}

		iFind = str.find("session_list:");//如果要获取的参数是session_list
		if(iFind != -1)
		{
			session_list = str.substr(iFind+13);//获取session list的信息
			printf("R6:%s\n",session_list.c_str());
		}

		return true;
	}
	else return false;
}
//解析announce消息
bool rtsp_r6_announce_msg_parse(string str)
{
	string::size_type iFind,iFindend;
	int cseq;
	_ERM_INT64 session;
	int msg_id;
	string msg;
	//string str = "ANNOUNCE rtsp://172.168.2.2/1234 RTSP/1.0\r\nCSeq: 316\r\nRequire: com.comcast.ngod.r6\r\nSession: 47112344\r\nNotice: 5401 \"Downstream Failure\"
	//event-date=19930316T064707.735Z npt=\r\nOnDemandSessionId: be074250-cc5a-11d9-8cd5-0800200c9a66\r\n";
	if(str.find("ANNOUNCE") != -1)
	{
		iFind = str.find("CSeq:");
		if(iFind != -1)
			cseq = atoi( str.substr(iFind+5).c_str());//Cseq号

		iFind = str.find("Session:");
		if ( iFind != -1 )
			session = atoll( str.substr(iFind+8).c_str() );//session号

		iFind = str.find("Notice:");
		msg_id = atoi( str.substr(iFind+7).c_str());//ANNOUNCE信息的ID

		iFind = str.find("\"");
		iFindend = str.find("\"",iFind+1);
		msg = str.substr(iFind+1,iFindend-iFind-1);//ANNOUNCE信息的内容
		printf("R6: %s\n%d  %llu %d %s\n",str.c_str(),cseq,session,msg_id,msg.c_str());
	}

	return true;
}

