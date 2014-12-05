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
