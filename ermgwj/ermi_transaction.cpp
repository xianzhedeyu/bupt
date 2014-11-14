#include"ermi.h"

#include"ermi_db_operate_module.h"
#include"erm_commnu_module.h"
#include"ermi_transaction.h"
#include"ermlog.h"
extern int LVLDEBUG;
int request_bw=0xFFFFFFF;
int pthread_Ermi_TP(char * Ermi_msg, int msg_len, int SM_sd) {
	string ret, msg;
	msg = Ermi_msg;
	ret = rtsp_ermi_response_parse(msg);
	if (ret == "SETUP") {
		fprintf(stderr, "------ERMi SETUP....\n");
		Ermi_Setup(Ermi_msg, msg_len, SM_sd);
	} else if (ret == "TEARDOWN") {
		fprintf(stderr, "------ERMi TEARDOWN....\n");
		Ermi_Teardown(Ermi_msg, msg_len, SM_sd);
	} else if (ret=="GET_PARAMETER"){
		fprintf(stderr, "------ERMi GETPARAMETER....\n");
		Ermi_GetParam(Ermi_msg, msg_len, SM_sd);
	} else {
		write(SM_sd, "TYPE ERROR", 20);
	}
	return 0;
}

int Ermi_Setup(char * Ermi_msg, int msg_len, int SM_sd) {
	log(LVLDEBUG,SYS_INFO,"Ermi_setup msg :%s len:%d\n",Ermi_msg,msg_len);
	char qam_addr[16] = "";
	int qam_port = 0;
	char sendbuf[1024];
	char recvbuf[1024];
	int qam_sd = 0;
	int buflen = 0;
	int i = 0, ret = 0;
	string buf = Ermi_msg;
	string rtspurl;

	string destination;
	int sm_cseq = 0;/*send SM*/
	int erm_cseq = 0;/* +1 send to qam */

	E_ERM_INT64 session_id = 0;
	int qam_cseq;
	E_ERM_INT64 qam_session_id;

	string str_notice;
	char tmp[256] = "";/*处理int型转为string型*/

	//SM发给ERM的SETUP消息
	ESETUP_MSG resp_setup_msg;

	//
	//Transport_QAM tqam;

	//Transport_UDP tudp;

	ESETUP_RESPONSE resp_ermi_msg;

	eqamsdinfo setup_static;
	int qam_numb = 1;
	int bandwidth = 0; /*可用带宽*/
	int Frequency = 0;
	int program = 0;
	int udp_port = 0;
	int err_code = 0;

	E_ERM_INT64 tmpsession = 10622555;

	//memset(&ann,0x00,sizeof(ANNOUNCE_MSG));
	memset(&setup_static, 0x0, sizeof(setup_static));
	//memset(&resp_setup_msg,0x00,sizeof(SETUP_MSG));
	//memset(&resp_ermi_msg,0x00,sizeof(resp_ermi_msg));
	//memset(&tqam,0x00,sizeof(Transport_QAM));
	//memset(&tudp,0x00,sizeof(Transport_UDP));

	/*setup S6 packet*/
	//解析 SM －> ERM 的setup消息 把结果参数存到resp_setup_msg中
	//cout << "bbbbbbbbbbbbbbbb" << endl;
	rtsp_ermi_setup_parse(buf, &resp_setup_msg);
	//cout << "bbbbbbbbbbbbbbbb" << endl;


	/***********************************/
	cout << "----------------------------------------------------------------" << endl;
    cout << resp_setup_msg.rtsp_url<<endl;
    cout << resp_setup_msg.cseq<<endl;
    cout << resp_setup_msg.require<<endl;
    cout << resp_setup_msg.qaminf[0].client << endl;
    cout << resp_setup_msg.qaminf[0].bit_rate << endl;
    cout << resp_setup_msg.qaminf[0].qam_id << endl;
    cout << resp_setup_msg.qaminf[0].depi_mode << endl;

    cout << resp_setup_msg.qaminf[1].client << endl;
    cout << resp_setup_msg.qaminf[1].bit_rate << endl;
    cout << resp_setup_msg.qaminf[1].qam_id << endl;
    cout << resp_setup_msg.qaminf[1].depi_mode << endl;
    cout << "----------------------------------------------------------------" << endl;
	/********************/

	sm_cseq = resp_setup_msg.cseq;

	// erm_cseq 是 sm_cseq ＋ 100
	erm_cseq = sm_cseq + 100;

	//SM 请求的 QAM 数量
	qam_numb = resp_setup_msg.qam_num;
	//	bandwidth=atoi(resp_setup_msg.qaminf->bandwidth.c_str());

	// qamselectinfo 封装的是 QAM 选定的 相关信息：比如 UDP/port 信息 Modulation/Frequency 等信息
	eqamselectinfo qam_info[qam_numb];

	//memset(&qam_info,0x00,sizeof(qamselectinfo)*qam_numb);
	for (i = 0; i < qam_numb; i++) {
		strncpy(qam_info[i].qam_name, resp_setup_msg.qaminf[i].qam_id.c_str(), MAX_STRING);
		//strncpy(qam_info[i].bit_rate, resp_setup_msg.qaminf[i].bit_rate.c_str(), MAX_STRING);
		request_bw = atoi(resp_setup_msg.qaminf[i].bit_rate.c_str());
	}
	//lock
	//根据 QAM 的name 从数据库中查询 满足条件的 QAM 信息

	fprintf(stderr, "------QAM_SELECT...\n");
	for (i = 0; i < qam_numb; i++) {
		fprintf(stderr, "------%s\n", qam_info[i].qam_name);
	}
//TOTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
	ret = EQAM_SELECT(qam_info, qam_numb);
//TOTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
	//unlock

	if (ret < 0) {
	/*	 如果没有找到合适的QAM资源则向SM发送 errno＝404 的错误，
		 * 发送出去就结束，
		 * 对于接收到的 SM 发回来的确认信息，
		 * 忽略处理*/

		//str_notice=get_announce_notice(ret);
		fprintf(stderr, "QAM_SELECT DB return %d\n", ret);
		log(LVLDEBUG,SYS_INFO,"QAM_SELECT DB return %d\n",ret);

		resp_ermi_msg.error_code = RTSP_ERMI_ResponseCode_NotFound ;
		resp_ermi_msg.cseq = sm_cseq;

		rtsp_ermi_setup_response_encode(resp_ermi_msg, sendbuf);
		rtsp_write(SM_sd, sendbuf, strlen(sendbuf));
		ret = rtsp_read(SM_sd, recvbuf, 1024);
		return -1;
	}

	char tbuf[1024];
	//sprintf(tbuf,"%d",qam_info[0].next_port);
	string header1 = "SETUP rtsp://" + string(qam_info[0].next_add) + "/" + string(qam_info[0].qam_name) + " RTSP/1.0\r\n";
	memset(tbuf,0,strlen(tbuf));
	sprintf(tbuf,"%d",erm_cseq);
	string cseq1 = "CSeq: "+ string(tbuf) +"\r\n";
	string require1 = "Require: ermi\r\n";
	string transport1 = "Transport: DOCSIS/QAM;" + resp_setup_msg.qaminf[0].client+"bit_rate="+resp_setup_msg.qaminf[0].bit_rate+";qam_id="+qam_info[0].qam_name + ";depi_mode=docsis_mpt\r\n\r\n";
	string msg = header1+cseq1+require1+transport1;
//SETUP rtsp://172.2.2.2/123 RTSP/1.0\r\n
//CSeq: 101\r\n
//Require: ermi\r\n
//Transport: DOCSIS/QAM;unicast;bit_rate=38000000;qam_id=123;depi_mode=docsis_mpt\r\n\r\n
	memset(sendbuf,0,strlen(sendbuf));
	memcpy(sendbuf,msg.c_str(),msg.size());
	cout << sendbuf << endl;

	//memcpy(qam_addr, qam_info[0].next_add, 16);
	//qam_port = qam_info[i].next_port;
// >>>> test
	//memcpy(qam_addr, string("127.0.0.1").c_str(), 16);
	memcpy(qam_addr, "127.0.0.1", 16);
	qam_port = 8765;
// <<<< test
	ret = ConnectSock(&qam_sd, qam_port, qam_addr);
	log(LVLSYS,SYS_INFO,"connecting...%s:%d ret:%d sd:%d\n",qam_addr,qam_port,ret,qam_sd);
	if (ret == -1) {
		log(LVLDEBUG,SYS_INFO,"Connect qam error");
		resp_ermi_msg.error_code = RTSP_ERMI_ResponseCode_NotFound;
		resp_ermi_msg.cseq = sm_cseq;
		//resp_ermi_msg.session=
		//resp_ermi_msg.onDemandSessionId = resp_setup_msg.ondemandsessionid;

		rtsp_ermi_setup_response_encode(resp_ermi_msg, sendbuf);
		rtsp_write(SM_sd, sendbuf, strlen(sendbuf));
		ret = rtsp_read(SM_sd, recvbuf, 1024);
		return -1;
	}

	//向 QAM 发送 配置信息
	rtsp_write(qam_sd, sendbuf, strlen(sendbuf));

	//读取 QAM 的相应信息
	ret = rtsp_read(qam_sd, recvbuf, 1024);

	log(LVLDEBUG,SYS_INFO,"qam setup response  msg:%s, len:%d\n",recvbuf,ret);

	buf = recvbuf;
	//检查预分配结果
	string str = string(buf);
	string tmpstr = str;
	int index = tmpstr.find("RTSP/1.0 ");
	tmpstr = tmpstr.substr(index+9);
	int index2 = tmpstr.find(" ");
	tmpstr = tmpstr.substr(0,index2);
	cout << tmpstr << endl;
	int code = atoi( tmpstr.c_str() );//解析出response codes

	index = str.find("CSeq:");
	int cseq;
	if ( index != string::npos )
	{
		cseq = atoi( str.substr(index+5).c_str() );//解析出cseq
	}
	cout << cseq << endl;
	index = str.find("Session:");
	long session;
	if ( index != string::npos )
	{
		session = atoll( str.substr(index+8).c_str() );//解析出session号
	}
	cout << session << endl;

	resp_ermi_msg.error_code = code;
	resp_ermi_msg.session = session;
	resp_ermi_msg.cseq = cseq + 100;
	resp_ermi_msg.qam_name = string(qam_info[0].qam_name);
	resp_ermi_msg.qam_destination = string(qam_info[0].next_add);
	cout << string(qam_info[0].next_add) << endl;

	strncpy(setup_static.qam_name, qam_info[0].qam_name, MAX_STRING);
	setup_static.udp_state = 2;

	//setup_static.use_bw = qam_info[0].available_bw;
	//test
	setup_static.use_bw = 100;
	snprintf(setup_static.qam_session, MAX_STRING, "%ld", session);
	//将 分配的 资源 存入数据库 表 Qam_udp???
	//lock
	ret = EQAM_SETUP_DOWN(setup_static);
	//unlock
	log(LVLDEBUG,SYS_INFO,"QAM_SETUP_DOWN return %d ,id:%s bw:%d\n",ret,setup_static.qam_session,qam_info[0].available_bw);

	//erm_cseq++;

	//QAM 响应校验 没有问题 开始组织数据 向 SM 发送分配的资源
	rtsp_ermi_setup_response_encode(resp_ermi_msg, sendbuf);
	cout << "sendbuf ====>>>>> " << sendbuf << endl;

	//memset(sendbuf,0x00,sizeof(sendbuf));
	//memset(recvbuf,0x00,sizeof(recvbuf));

	buflen = strlen(sendbuf);
	log(LVLDEBUG,SYS_INFO,"erm2SM setup  response  msg:%s, len:%d\n",sendbuf,buflen);

	//通知 SM 资源分配完毕
	rtsp_write(SM_sd, sendbuf, buflen);
/*
	//读取 SM 响应
	ret = rtsp_read(SM_sd, recvbuf, 1024);

	log(LVLDEBUG,SYS_INFO,"recv SM  msg:%s, len:%d\n",recvbuf,ret);
	if (ret > 0) {
		Ermi_Teardown(recvbuf, ret, SM_sd);
	} else {
		fprintf(stderr, "SM close his socket\n");
	}*/
	close(qam_sd);
	return 0;
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
