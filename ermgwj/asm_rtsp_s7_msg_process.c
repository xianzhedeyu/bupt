#include "asm_rtsp_s7_msg_process.h"
#include "asm_rtsp_public_function.h"

int rtsp_s7_setup_res_encode(S7_SETUP_RES resmsg, char *setup_res_msg)
{
	char str[1024]="":
	char transport[256]="";
	
	memset(transport, 0x00, sizeof(transport));
	sprintf(transport, "MP2T/DVBC/UDP;unicast;client=%s;destination=%s;client_port=%d", resmsg.ss.client, resmsg.ss.destination, resmsg.ss.client_port);
	memset(str,0x00,sizeof(str));
	if(resmsg.err_code==200){
		sprintf(str,"RTSP/1.0 %d OK\r\n"
			"CSeq:%d\r\n"
			"Session:%llu\r\n"
			"OnDemandSessionId:%s\r\n"
			"Transport:%s\r\n"
			"AS_IP:%s\r\n"
			"AS_PORT:%d\r\n"
			resmsg.err_code, resmsg.cseq, resmsg.session, resmsg.ondemand_session_id, transport, resmsg.as.as_ip, resmsg.as.as_port);			
}
	strcpy(setup_res_msg,str);
	return 0;
}
int rtsp_s7_setup_msg_parse(char *setup_msg, S7_SETUP_MSG msg)
{
}
