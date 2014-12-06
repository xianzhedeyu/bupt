#include"asm_commnunication_module.h"
#include"asm_transaction.h"
#include"asmlog.h"
extern int LVLDEBUG;
int request_bw=0xFFFFFFF;
int pthread_Asm_TP(char * Asm_msg, int msg_len, int asm_sd) {
	string ret, msg;
	msg = Asm_msg;
	ret = rtsp_Asm_response_parse(msg);
	if (ret == "SETUP") {
		fprintf(stderr, "------ASM SETUP....\n");
		Asm_Setup(Asm_msg, msg_len, asm_sd);
	} else if (ret == "TEARDOWN") {
		fprintf(stderr, "------ASM TEARDOWN....\n");
		Asm_Teardown(Asm_msg, msg_len, asm_sd);
	} else if (ret=="GET_PARAMETER"){
		fprintf(stderr, "------ASM GETPARAMETER....\n");
		Asm_GetParam(Asm_msg, msg_len, asm_sd);
	} else {
		write(Asm_sd, "TYPE ERROR", 20);
	}
	return 0;
}

int Asm_Setup(char * Asm_msg, int msg_len, int asm_sd) {
	log(LVLDEBUG,SYS_INFO,"sm_setup msg :%s len:%d\n",Asm_msg,msg_len);
	char as_addr[40] = "";
	int as_port = 0;
	char sendbuf[1024];
	char recvbuf[1024];
	int as_sd = 0;
	int buflen = 0;
	int i = 0, ret = 0;
	char* buf = Asm_msg;

    R8_SETUP_MSG r8_setup_msg;
    R8_SETUP_RES r8_setup_res;
    S7_SETUP_MSG s7_setup_msg;
    S7_SETUP_RES s7_setup_res;

    INT64 asm2as
}


int Ermi_Teardown(char * Ermi_msg, int msg_len, int SM_sd) {

	log(LVLDEBUG,SYS_INFO,"SM teardown(step 1) msg:%s, len:%d\n",Ermi_msg,msg_len);

	string str = Ermi_msg;
	char buf[1024] = "";
	string strbuf;
	char sendbuf[1024];
	eqamsdinfo qma_sd;
	int sm_cseq, erm_cseq;
	int qam_port;
	int ret;
	char qam_addr[16] = "";
	int qam_sd;
	//Transport_QAM tqam;
	ETEARDOWN_MSG_RES resp_ermi_msg;

	int qam_cseq;
	E_ERM_INT64 qam_session_id;
	string qam_ondemandsessionid;
	eqamselectinfo_down qs_qam;
	eqamsdinfo qam_static;
	int err_code = 0;
	//memset(&ann,0x00,sizeof(ann));
	memset(&qs_qam, 0x0, sizeof(qs_qam));
	memset(&qam_static, 0x00, sizeof(qam_static));
	//SM requst teardown to erm: step 1
	ETEARDOWN_MSG sm2erm;
	//memset(&sm2erm,0x00,sizeof(sm2erm));

	rtsp_ermi_teardown_parse(str, &sm2erm);
	cout << sm2erm.cseq << endl;
	cout << sm2erm.require << endl;
	cout << sm2erm.rtsp_url << endl;
	cout << sm2erm.session << endl;

	sm_cseq = sm2erm.cseq;
	erm_cseq = sm_cseq + 100;
	sprintf(qs_qam.qam_session,"%lld",sm2erm.session);
	cout << "qs_qam.qam_session:" << qs_qam.qam_session << endl;
	//memcpy(qs_qam.qam_session, sm2erm.session, MAX_STRING);
	//lock
	ret = EQAM_DOWN_SELECT(&qs_qam);
	//unlock
	log(LVLDEBUG,SYS_INFO,"QAM_DOWN_SELECT return %d,qam_session %s\n,qam_name %s usebw %d port %d add %s\n",ret,qs_qam.qam_session,qs_qam.qam_name,qs_qam.use_bw,qs_qam.next_port,qs_qam.next_add);
	if (ret) {
		resp_ermi_msg.error_code = RTSP_ERMI_ResponseCode_NotFound;
		resp_ermi_msg.cseq = sm_cseq;
		resp_ermi_msg.session = sm2erm.session;

		rtsp_ermi_teardown_res_encode(resp_ermi_msg, sendbuf);
		log(LVLDEBUG,SYS_INFO,"Connect qam error,addr:%s port:%d\n",qam_addr,qam_port);
		rtsp_write(SM_sd, sendbuf, strlen(sendbuf));
		ret = rtsp_read(SM_sd, buf, 1024);
		return -1;
	}
	qam_port = qs_qam.next_port;
	memcpy(qam_addr, qs_qam.next_add, 16);

	strncpy(qam_static.qam_name, qs_qam.qam_name, MAX_STRING);
	qam_static.udp_port = qs_qam.next_port;
	qam_static.use_bw = qs_qam.use_bw;
	strncpy(qam_static.qam_session, qs_qam.qam_session, MAX_STRING);

//	ann.rtspurl = qam_addr;
//	ann.cseq = sm_cseq;
//	ann.session = sm2erm.session;
//	ann.ondemandsessionid = sm2erm.ondemandsessionid;

	ret = ConnectSock(&qam_sd, qam_port, qam_addr);

	if (ret < 0) {
		resp_ermi_msg.error_code = RTSP_ERMI_ResponseCode_NotFound;
		resp_ermi_msg.cseq = sm_cseq;
		resp_ermi_msg.session = sm2erm.session;
		rtsp_ermi_teardown_res_encode(resp_ermi_msg, sendbuf);
		log(LVLDEBUG,SYS_INFO,"Connect qam error,addr:%s port:%d\n",qam_addr,qam_port);
		rtsp_write(SM_sd, sendbuf, strlen(sendbuf));
		ret = rtsp_read(SM_sd, buf, 1024);
		return -1;
	}

	string tport;
	char atport[128];
	sprintf(atport,"%d",qam_port);
	tport=string(atport);
	string turl = string(qam_addr) + "/" + tport;
	//erm requst teardown to qam: step 2
	ETEARDOWN_MSG erm2qam;
	memset(&erm2qam,0x00,sizeof(erm2qam));
	memcpy(erm2qam.rtsp_url,turl.c_str(),turl.size());
	//erm2qam.rtsp_url = qam_addr;
	erm2qam.cseq = sm2erm.cseq  + 100;
	erm2qam.session = sm2erm.session + 100;

	rtsp_ermi_teardown_encode(erm2qam, sendbuf);

	//memset(sendbuf,0x00,sizeof(sendbuf));
	//memset(buf,0x00,sizeof(buf));
	log(LVLDEBUG,SYS_INFO,"erm2QAM teardown(step 2) msg:%s, len:%d\n",sendbuf,strlen(sendbuf));

	rtsp_write(qam_sd, sendbuf, strlen(sendbuf));
	ret = rtsp_read(qam_sd, buf, 1024);


	log(LVLDEBUG,SYS_INFO,"QAM2erm teardown(step 3) response(2) msg:%s, len:%d\n",buf,ret);

	// sm response step 2 teardown
	ETEARDOWN_MSG_RES qam_resp2erm;
	str = string(buf);

	rtsp_ermi_teardown_res_parse(str, &qam_resp2erm);
	log(LVLDEBUG,SYS_INFO,"QAM2erm teardown(step 3) response(2) parse:errcode: %d, cseq:%d session:%d\n",qam_resp2erm.error_code,qam_resp2erm.cseq,qam_resp2erm.session);

	qam_static.udp_state = 3;

	//lock
	ret = EQAM_SETUP_DOWN(qam_static);
	//unlock
	log(LVLDEBUG,SYS_INFO,"QAM_SETUP_DOWN return %d\n",ret);
	if (ret) {
	}
	//to qam
	//tqam 有问题 没有初始化
	ETEARDOWN_MSG_RES erm2cm;
	erm2cm.cseq = sm2erm.cseq;
	erm2cm.session = sm2erm.session;
	erm2cm.error_code = 200;
	//erm2cm.rtsp_url = sm2erm.rtsp_url;
	//memcpy(erm2cm.rtsp_url, sm2erm.rtsp_url, strlen(sm2erm.rtsp_url) + 1);
	//erm2cm.require = sm2erm.require;
	//memcpy(erm2cm.require, sm2erm.require, strlen(sm2erm.require)+1);

	rtsp_ermi_teardown_res_encode(erm2cm, sendbuf);
	//memset(sendbuf,0x00,sizeof(sendbuf));
	//memset(buf,0x00,sizeof(buf));
	//memcpy(sendbuf, str.c_str(), str.length());
	log(LVLDEBUG,SYS_INFO,"erm2qam teardown(step 4)  msg:%s, len:%d\n",sendbuf,str.length());

	rtsp_write(SM_sd, sendbuf, strlen(sendbuf) + 1);
	ret = rtsp_read(SM_sd, buf, 1024);

	log(LVLDEBUG,SYS_INFO,"qam2erm teardown(step 5) response(4) msg:%s, len:%d\n",buf,ret);
/*
	str = buf;
	if (!GetResponses(str, &err_code, &qam_cseq, &qam_session_id, &qam_ondemandsessionid)) {
		log(LVLDEBUG,SYS_INFO,"Qam response abnormity\n");

		resp_ermi_msg.error_code = err_code;
		resp_ermi_msg.session = sm2erm.session;
		resp_ermi_msg.onDemandSessionId = sm2erm.ondemandsessionid;
		resp_ermi_msg.cseq = sm_resp2erm.cseq;

		rtsp_ermi_setup_response_encode(resp_ermi_msg, sendbuf);
		rtsp_write(SM_sd, sendbuf, strlen(sendbuf));
		ret = rtsp_read(SM_sd, buf, 1024);
		return -1;

		 ann.cseq=sm_resp2erm.cseq;
		 strbuf=rtsp_s6_teardown_res_encode(ann);
		 memcpy(sendbuf,strbuf.c_str(),strbuf.length());
		 rtsp_write(SM_sd,sendbuf,strbuf.length());
		 ret=rtsp_read(SM_sd,buf,1024);
		 return 0;

		通知sm所选qam不可用
	}

	qam_static.udp_state = 4;
	teardown pre4
	//lock
	QAM_SETUP_DOWN(qam_static);
	//unlock

	erm response step 1 to sm
	TEARDOWN_RES2 erm_resp2sm;
	//memset(&erm_resp2sm,0x00,sizeof(erm_resp2sm));
	memcpy(&erm_resp2sm, &sm_resp2erm, sizeof(erm_resp2sm));
	erm_resp2sm.cseq = sm_cseq;
	rtsp_s6_teardown_res_encode(erm_resp2sm, sendbuf);

	//memset(sendbuf,0x00,sizeof(sendbuf));
	//memset(buf,0x00,sizeof(buf));
	log(LVLDEBUG,SYS_INFO,"erm2SM teardown(step 6) response(step1) msg:%s, len:%d\n",sendbuf,strlen(sendbuf));

	rtsp_write(SM_sd, sendbuf, strlen(sendbuf));

	log(LVLDEBUG,SYS_INFO,"write over\n");

	ret = rtsp_read(SM_sd, buf, 1024);

	log(LVLDEBUG,SYS_INFO,"read over\n");*/
	close(qam_sd);
	return 0;

}
int Ermi_GetParam(char * Ermi_msg, int msg_len, int SM_sd) {
	cout << "get param " << endl;
	cout << Ermi_msg << endl;
	EGETPARAM_MSG getparam;
	rtsp_ermi_getparam_parse(string(Ermi_msg), &getparam);
//	cout << getparam.rtsp_url << endl;
//	cout << "1----" << endl;
//	cout << getparam.cseq << endl;
//	cout << "2----" << endl;
//	cout << getparam.ctype << endl;
//	cout << "3----" << endl;
//	cout << getparam.clen << endl;
//	cout << "4----" << endl;
//	cout << getparam.param << endl;
//	cout << "5----" << endl;
//	cout << getparam.require << endl;
	string res;
	EQAM_SESSION_SELECT(getparam, res);
	cout << "result: " << res << endl;
	EGETPARAM_MSG getparamresp;
	string resp;
	getparamresp.rtsp_url = getparam.rtsp_url;
	getparamresp.cseq = getparam.cseq + 10;
	getparamresp.ctype = getparam.ctype;
	getparamresp.clen = res.size() + 2;
	getparamresp.param = res;
	rtsp_ermi_getparm_encode(getparamresp, resp);
	cout << "resp: " << "-------------------" << endl;
	cout << resp << endl;
	cout << "-------------------------" << endl;
	char buff[1024];
	memset(buff, 0, 1024);
	memcpy(buff, resp.c_str(), resp.size());
	rtsp_write(SM_sd, buff, resp.size());
	/*
	 * RTSP/1.0 200 OK\r\n
CSeq: 130\r\n
Content-length: 31\r\n
Content-type: text/parameters\r\n\r\n
session_list: 1234567 1234568\r\n
	 */

}
