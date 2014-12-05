/*
 版本:v1.0

 * 作者:王传华

 * 创建时间:2010-08-25

 *修改记录:

 *程序说明:
 S6 R6 业务处理

 *输入参数: 无

 *输出参数: 无

 *返回值:无
 */
#include"erm_rtsp_s6_msg_process.h"
#include"erm_rtsp_r6_msg_process.h"
#include"erm_db_operate_module.h"
#include"erm_commnu_module.h"
#include"erm_transaction.h"
#include"ermlog.h"
extern int LVLDEBUG;
int pthread_TP(char * R6_msg, int msg_len, int SM_sd) {
	string ret, msg;
	msg = R6_msg;
	ret = rtsp_s6__response_parse(msg);
	if (ret == "SETUP") {
		fprintf(stderr, "------ERM S6 SETUP....\n");
		Tp_Setup(R6_msg, msg_len, SM_sd);
	} else if (ret == "TEARDOWN") {
		Tp_Teardown(R6_msg, msg_len, SM_sd);
	} else {
		write(SM_sd, "TYPE ERROR", 20);
	}
	return 0;
}

int Tp_Setup(char * R6_msg, int msg_len, int SM_sd) {
	log(LVLDEBUG,SYS_INFO,"R6_setup msg :%s len:%d\n",R6_msg,msg_len);
	char qam_addr[16] = "";
	int qam_port = 0;
	char sendbuf[1024];
	char recvbuf[1024];
	int qam_sd = 0;
	int buflen = 0;
	int i = 0, ret = 0;
	string buf = R6_msg;
	string rtspurl;
	string inband_marker;
	string destination;
	int sm_cseq = 0;/*send SM*/
	int erm_cseq = 0;/* +1 send to qam*/
	char ondemandsessionid[256] = "";
	_ERM_INT64 session_id = 0;
	int qam_cseq;
	_ERM_INT64 qam_session_id;
	string qam_ondemandsessionid;

	string str_notice;
	char tmp[256] = "";/*处理int型转为string型*/

	//SM发给ERM的SETUP消息
	SETUP_MSG resp_setup_msg;

	//
	Transport_QAM tqam;


	Transport_UDP tudp;


	SETUP_RESPONSE resp_s6_msg;

	//ERM给SM发的announce消息
	ANNOUNCE_MSG ann;

	qamsdinfo setup_static;
	int qam_numb = 1;
	int bandwidth = 0; /*可用带宽*/
	int Frequency = 0;
	int program = 0;
	int udp_port = 0;
	int err_code = 0;

	_ERM_INT64 tmpsession = 10622555;

	//memset(&ann,0x00,sizeof(ANNOUNCE_MSG));	
	memset(&setup_static, 0x0, sizeof(setup_static));
	//memset(&resp_setup_msg,0x00,sizeof(SETUP_MSG));
	//memset(&resp_s6_msg,0x00,sizeof(resp_s6_msg));
	//memset(&tqam,0x00,sizeof(Transport_QAM));
	//memset(&tudp,0x00,sizeof(Transport_UDP));

	/*setup S6 packet*/
	//解析 SM －> ERM 的setup消息 把结果参数存到resp_setup_msg中
	rtsp_s6_setup_msg_parse(buf, &resp_setup_msg);

	//从 S6 的消息中解析出 Announce 消息需要的信息
	ann.rtspurl = resp_setup_msg.rtsp_url;
	ann.cseq = resp_setup_msg.cseq;
	ann.session = tmpsession;	//session 是临时的？？
	ann.ondemandsessionid = resp_setup_msg.ondemandsessionid;

	strncpy(ondemandsessionid, resp_setup_msg.ondemandsessionid.c_str(), 256);
	log(LVLDEBUG,SYS_INFO,"********** %s **********\n",resp_setup_msg.ondemandsessionid.c_str());

	sm_cseq = resp_setup_msg.cseq;

	// erm_cseq 是 sm_cseq ＋ 100
	erm_cseq = sm_cseq + 100;
	inband_marker = resp_setup_msg.inband_marker;

	//SM 请求的 QAM 数量
	qam_numb = resp_setup_msg.qam_num;
	//	bandwidth=atoi(resp_setup_msg.qam_info->bandwidth.c_str());

	// qamselectinfo 封装的是 QAM 选定的 相关信息：比如 UDP/port 信息 Modulation/Frequency 等信息
	qamselectinfo qam_info[qam_numb];

	//memset(&qam_info,0x00,sizeof(qamselectinfo)*qam_numb);
	for (i = 0; i < qam_numb; i++) {
		strncpy(qam_info[i].qam_name,
				resp_setup_msg.qam_info[i].qam_name.c_str(), MAX_STRING);
	}
	//lock
//根据 QAM 的name 从数据库中查询 满足条件的 QAM 信息

	fprintf(stderr, "------QAM_SELECT...\n");
	for (i = 0; i < qam_numb; i++) {
			fprintf(stderr,"------%s\n",qam_info[i].qam_name);
	}

	ret = QAM_SELECT(qam_info, qam_numb);

	//unlock
	if (ret < 0) {
		/* 如果没有找到合适的QAM资源则向SM发送 errno＝404 的错误，
		 * 发送出去就结束，
		 * 对于接收到的 SM 发回来的确认信息，
		 * 忽略处理
		 */
		//str_notice=get_announce_notice(ret);
		fprintf(stderr,"QAM_SELECT DB return %d\n",ret);
		log(LVLDEBUG,SYS_INFO,"QAM_SELECT DB return %d\n",ret);

		resp_s6_msg.error_code = RTSP_S6_ResponseCode_NotFound;
		resp_s6_msg.cseq = sm_cseq;
		//resp_s6_msg.session=	
		resp_s6_msg.onDemandSessionId = resp_setup_msg.ondemandsessionid;

		rtsp_s6_setup_response_encode(resp_s6_msg, sendbuf);
		rtsp_write(SM_sd, sendbuf, strlen(sendbuf));
		ret = rtsp_read(SM_sd, recvbuf, 1024);
		return -1;
	}
	/*选可用的qam*/
	/*这里可用 QAM 的选取 只是简单的比较下 QAM 的调制方式 与 带宽 是否 满足 SM 请求的条件，如果是，则直接使用此 QAM */
	/* 这里是需要用 算法 选择的*/
	for (i = 0; i < qam_numb; i++) {
		log(LVLDEBUG,SYS_INFO,"available :%d mod:%s\n",qam_info[i].available_bw,qam_info[i].Modmode);
		log(LVLDEBUG,SYS_INFO,"resp_setup_msg.qam_info[i].bandwidth.c_str():%s\n"
							"resp_setup_msg.qam_info[i].modulation.c_str():%s\n",resp_setup_msg.qam_info[i].bandwidth.c_str(),resp_setup_msg.qam_info[i].modulation.c_str());
		if (qam_info[i].available_bw >= atoi(
				resp_setup_msg.qam_info[i].bandwidth.c_str()) && strcmp(
				qam_info[i].Modmode,
				resp_setup_msg.qam_info[i].modulation.c_str()) == 0)
		{

			break;
		}

	}
	if (i == qam_numb) {
		log(LVLDEBUG,SYS_INFO,"Unmatching qam's bandwidth or qam's modulation in %d's qam\n",qam_numb);
		resp_s6_msg.error_code = RTSP_S6_ResponseCode_NotFound;
		resp_s6_msg.cseq = sm_cseq;
		//resp_s6_msg.session=	
		resp_s6_msg.onDemandSessionId = resp_setup_msg.ondemandsessionid;

		rtsp_s6_setup_response_encode(resp_s6_msg, sendbuf);
		rtsp_write(SM_sd, sendbuf, strlen(sendbuf));
		ret = rtsp_read(SM_sd, recvbuf, 1024);
		return -1;
		/*通知sm所选qam不可用*/
	}

	if (atoi(qam_info[i].input_add) == 0)
		strcpy(qam_info[i].input_add, "0.0.0.0");

/*目的QAM的频率和节目号*/
	fprintf(stderr,"DB return infomation: qam_name:%s,qam_add:%s,available_bw:%d,udp_num:%d,input_add:%s,input_port:%d,next_add:%s,next_port:%d,Frequency:%d,qam_group:%s,input_group:%s,modmode:%s\n",
				qam_info[i].qam_name,qam_info[i].qam_add,qam_info[i].available_bw,qam_info[i].udp_num,qam_info[i].input_add,qam_info[i].input_port,qam_info[i].next_add,qam_info[i].next_port,qam_info[i].Frequency,qam_info[i].qam_group,qam_info[i].input_group,qam_info[i].Modmode);
	log(LVLDEBUG,SYS_INFO,"DB return infomation: qam_name:%s,qam_add:%s,available_bw:%d,udp_num:%d,input_add:%s,input_port:%d,next_add:%s,next_port:%d,Frequency:%d,qam_group:%s,input_group:%s,modmode:%s\n",
			qam_info[i].qam_name,qam_info[i].qam_add,qam_info[i].available_bw,qam_info[i].udp_num,qam_info[i].input_add,qam_info[i].input_port,qam_info[i].next_add,qam_info[i].next_port,qam_info[i].Frequency,qam_info[i].qam_group,qam_info[i].input_group,qam_info[i].Modmode);

	udp_port = qam_info[i].udp_program[5][0];
	program = qam_info[i].udp_program[5][1];

	//test
	//udp_port=261;

	/* 需要通知 STB 接收的 频率 和 节目号，这个信息会 发送个 SM */
	sprintf(tmp, "%ld.%d", qam_info[i].Frequency * 1000, program);
	destination = tmp;

	/*connect arg*/
	memcpy(qam_addr, qam_info[i].next_add, 16);
	qam_port = qam_info[i].next_port;

	/*预分配 R6*/
	rtspurl = qam_addr;
	tqam.qam_destination = destination;
	// test
	//sprintf(tmp, "%d", qam_info[i].available_bw + 2700000);
	sprintf(tmp,"%d",qam_info[i].available_bw);

	tqam.bandwidth = tmp;

	tqam.qam_name = qam_info[i].qam_name;
	tqam.client = "FFFFFFFFFFFF";
	/*被推流服务器的地址*/

	tudp.destination = qam_info[i].input_add;

	//memset(tmp,0x0,sizeof(tmp));
	sprintf(tmp, "%d", udp_port);

	tudp.client_port = tmp;
	/*视频服务器的地址，以后需要更改*/
	tudp.source = "0.0.0.0";

	//memset(tmp,0x0,sizeof(tmp));
	sprintf(tmp, "%d", qam_info[i].input_port);

	tudp.server_port = tmp;
	tudp.client = "FFFFFFFFFFFF";
	/*response SM packet*/
	resp_s6_msg.cseq = sm_cseq;
	/*session_id在下面产生
	 resp_s6_msg.session=session_id;
	 ********/
	resp_s6_msg.client = resp_setup_msg.qam_info[i].client;
	resp_s6_msg.qam_destination = destination;
	resp_s6_msg.destination = qam_addr;
	resp_s6_msg.client_port = qam_port;
	resp_s6_msg.qam_name = qam_info[i].qam_name;
	resp_s6_msg.qam_group = qam_info[i].qam_group;
	resp_s6_msg.modulation = resp_setup_msg.qam_info[i].modulation;
	resp_s6_msg.edge_input_group = qam_info[i].input_group;
	resp_s6_msg.onDemandSessionId = resp_setup_msg.ondemandsessionid;

	/**/
	string r6_resp = rtsp_r6_setup_provision_port_msg_encode(rtspurl, erm_cseq,
			tqam, tudp, 1, inband_marker, 20);

	buflen = r6_resp.length();

	ret = ConnectSock(&qam_sd, qam_port, qam_addr);

	if (ret == -1) {
		log(LVLDEBUG,SYS_INFO,"Connect qam error");
		resp_s6_msg.error_code = RTSP_S6_ResponseCode_NotFound;
		resp_s6_msg.cseq = sm_cseq;
		//resp_s6_msg.session=	
		resp_s6_msg.onDemandSessionId = resp_setup_msg.ondemandsessionid;

		rtsp_s6_setup_response_encode(resp_s6_msg, sendbuf);
		rtsp_write(SM_sd, sendbuf, strlen(sendbuf));
		ret = rtsp_read(SM_sd, recvbuf, 1024);
		return -1;
		/*
		 buf=rtsp_s6_teardown_res_encode(ann);
		 memcpy(sendbuf,buf.c_str(),buf.length());
		 rtsp_write(SM_sd,sendbuf,buf.length());
		 ret=rtsp_read(SM_sd,recvbuf,1024);
		 return 0;
		 */
		/*通知sm所选qam不可用*/
	}
	//memset(sendbuf,0x00,sizeof(sendbuf));
	//memset(recvbuf,0x00,sizeof(recvbuf));


	memcpy(sendbuf, r6_resp.c_str(), buflen);
	log(LVLDEBUG,SYS_INFO,"setup send2qam msg:%s, len:%d\n",sendbuf,buflen);

	/*向 QAM 发送 配置信息*/
	rtsp_write(qam_sd, sendbuf, buflen);
	/*读取 QAM 的相应信息*/
	ret = rtsp_read(qam_sd, recvbuf, 1024);

	log(LVLDEBUG,SYS_INFO,"qam setup response  msg:%s, len:%d\n",recvbuf,ret);

	buf = recvbuf;
	/*检查预分配结果*/

	if (!GetResponses(buf, &err_code, &qam_cseq, &qam_session_id,
			&qam_ondemandsessionid)) {
		resp_s6_msg.error_code = err_code;
		resp_s6_msg.cseq = sm_cseq;
		//resp_s6_msg.session=	
		resp_s6_msg.onDemandSessionId = resp_setup_msg.ondemandsessionid;

		rtsp_s6_setup_response_encode(resp_s6_msg, sendbuf);
		rtsp_write(SM_sd, sendbuf, strlen(sendbuf));
		ret = rtsp_read(SM_sd, recvbuf, 1024);
		return -1;
		/*
		 //rtsp_s6_error_res_encode(err_code,qam_cseq);
		 ann.cseq=resp_setup_msg.cseq;
		 ann.session=qam_session_id;
		 log(LVLDEBUG,SYS_INFO,"Qam response abnormity");
		 buf=rtsp_s6_teardown_res_encode(ann);
		 memcpy(sendbuf,buf.c_str(),buf.length());
		 rtsp_write(SM_sd,sendbuf,buf.length());
		 ret=rtsp_read(SM_sd,recvbuf,1024);
		 return 0;
		 */
		/*通知sm所选qam不可用*/
	}
	/*取回会话id*/

	resp_s6_msg.session = qam_session_id;
	strncpy(setup_static.qam_name, qam_info[i].qam_name, MAX_STRING);
	setup_static.udp_port = udp_port;
	setup_static.program_id = program;
	setup_static.udp_state = 2;
	setup_static.use_bw = qam_info[i].available_bw;
	snprintf(setup_static.qam_session, MAX_STRING, "%ld", qam_session_id);
	strncpy(setup_static.ondemand_session,
			resp_setup_msg.ondemandsessionid.c_str(), MAX_STRING);
/*将 分配的 资源 存入数据库 表 Qam_udp???*/
	//lock
	ret = QAM_SETUP_DOWN(setup_static);
	//unlock

	log(LVLDEBUG,SYS_INFO,"QAM_SETUP_DOWN return %d ,id:%s bw:%d\n",ret,setup_static.qam_session,qam_info[i].available_bw);

	erm_cseq++;
	/*添加 QAM 校验 信息*/
	r6_resp = rtsp_r6_setup_start_checking_msg_encode(rtspurl, erm_cseq,
			qam_session_id, tqam, tudp, 3, inband_marker, 200);
	//memset(sendbuf,0x00,sizeof(sendbuf));
	//memset(recvbuf,0x00,sizeof(recvbuf));

	memcpy(sendbuf, r6_resp.c_str(), buflen);

	log(LVLDEBUG,SYS_INFO,"erm2qam setup start_checking msg:%s, len:%d\n",sendbuf,buflen);

	/* 通知 QAM 要校验的数据*/
	rtsp_write(qam_sd, sendbuf, buflen);

	/* 接收 QAM 的响应 */
	ret = rtsp_read(qam_sd, recvbuf, 1024);

	log(LVLDEBUG,SYS_INFO,"qam setup start_checking response  msg:%s, len:%d\n",recvbuf,ret);

	buf = recvbuf;
	if (!GetResponses(buf, &err_code, &qam_cseq, &qam_session_id,
			&qam_ondemandsessionid)) {
		resp_s6_msg.error_code = err_code;
		resp_s6_msg.cseq = sm_cseq;
		//resp_s6_msg.session=	
		resp_s6_msg.onDemandSessionId = resp_setup_msg.ondemandsessionid;

		rtsp_s6_setup_response_encode(resp_s6_msg, sendbuf);
		rtsp_write(SM_sd, sendbuf, strlen(sendbuf));
		ret = rtsp_read(SM_sd, recvbuf, 1024);
		return -1;
		/*
		 ann.cseq=resp_setup_msg.cseq;
		 ann.session=tmpsession;
		 log(LVLDEBUG,SYS_INFO,"Qam response abnormity");
		 buf=rtsp_s6_teardown_res_encode(ann);
		 memcpy(sendbuf,buf.c_str(),buf.length());
		 rtsp_write(SM_sd,sendbuf,buf.length());
		 ret=rtsp_read(SM_sd,recvbuf,1024);
		 return 0;
		 */
		/*通知sm所选qam不可用*/
	}


	/*QAM 响应校验 没有问题 开始组织数据 向 SM 发送分配的资源*/
	rtsp_s6_setup_response_encode(resp_s6_msg, sendbuf);

	//memset(sendbuf,0x00,sizeof(sendbuf));
	//memset(recvbuf,0x00,sizeof(recvbuf));

	buflen = strlen(sendbuf);
	log(LVLDEBUG,SYS_INFO,"erm2SM setup  response  msg:%s, len:%d\n",sendbuf,buflen);

	/* 通知 SM 资源分配完毕 */
	rtsp_write(SM_sd, sendbuf, buflen);

	/* 读取 SM 响应 */
	ret = rtsp_read(SM_sd, recvbuf, 1024);

	log(LVLDEBUG,SYS_INFO,"recv SM  msg:%s, len:%d\n",recvbuf,ret);
	if (ret > 0) {
		Tp_Teardown(recvbuf, ret, SM_sd);
	} else {
		fprintf(stderr, "SM close his socket\n");
	}
	close(qam_sd);
	return 0;
}
int Tp_Teardown(char * R6_msg, int msg_len, int SM_sd) {
	log(LVLDEBUG,SYS_INFO,"SM teardown(step 1) msg:%s, len:%d\n",R6_msg,msg_len);
	string str = R6_msg;
	char buf[1025] = "";
	string strbuf;
	char sendbuf[1025];
	qamsdinfo qma_sd;
	int sm_cseq, erm_cseq;
	int qam_port;
	int ret;
	char qam_addr[16] = "";
	int qam_sd;
	ANNOUNCE_MSG ann;
	Transport_QAM tqam;
	SETUP_RESPONSE resp_s6_msg;
	int qam_cseq;
	_ERM_INT64 qam_session_id;
	string qam_ondemandsessionid;
	qamselectinfo_down qs_qam;
	qamsdinfo qam_static;
	int err_code = 0;
	//memset(&ann,0x00,sizeof(ann));
	memset(&qs_qam, 0x0, sizeof(qs_qam));
	memset(&qam_static, 0x00, sizeof(qam_static));
	/*SM requst teardown to erm: step 1*/
	TEARDOWN_MSG1 sm2erm;
	//memset(&sm2erm,0x00,sizeof(sm2erm));

	rtsp_s6_teardown_msg_parse(str, &sm2erm);

	sm_cseq = sm2erm.cseq;
	erm_cseq = sm_cseq + 100;

	memcpy(qs_qam.ondemand_session, sm2erm.ondemandsessionid, MAX_STRING);
	//lock
	ret = QAM_DOWN_SELECT(&qs_qam);
	//unlock
	log(LVLDEBUG,SYS_INFO,"QAM_DOWN_SELECT return %d,ondemand %s\n",ret,qs_qam.ondemand_session);
	if (ret) {
		resp_s6_msg.error_code = RTSP_S6_ResponseCode_NotFound;
		resp_s6_msg.cseq = sm_cseq;
		resp_s6_msg.session = sm2erm.session;
		resp_s6_msg.onDemandSessionId = sm2erm.ondemandsessionid;

		rtsp_s6_setup_response_encode(resp_s6_msg, sendbuf);
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
	strncpy(qam_static.ondemand_session, qs_qam.ondemand_session, MAX_STRING);

	ann.rtspurl = qam_addr;
	ann.cseq = sm_cseq;
	ann.session = sm2erm.session;
	ann.ondemandsessionid = sm2erm.ondemandsessionid;

	ret = ConnectSock(&qam_sd, qam_port, qam_addr);

	if (ret < 0) {
		resp_s6_msg.error_code = RTSP_S6_ResponseCode_NotFound;
		resp_s6_msg.cseq = sm_cseq;
		resp_s6_msg.session = sm2erm.session;
		resp_s6_msg.onDemandSessionId = sm2erm.ondemandsessionid;

		rtsp_s6_setup_response_encode(resp_s6_msg, sendbuf);
		log(LVLDEBUG,SYS_INFO,"Connect qam error,addr:%s port:%d\n",qam_addr,qam_port);
		rtsp_write(SM_sd, sendbuf, strlen(sendbuf));
		ret = rtsp_read(SM_sd, buf, 1024);
		return -1;
		/*
		 strbuf=rtsp_s6_teardown_res_encode(ann);
		 memcpy(sendbuf,strbuf.c_str(),strbuf.length());
		 rtsp_write(SM_sd,sendbuf,strbuf.length());
		 ret=rtsp_read(SM_sd,buf,1024);
		 return 0;
		 */
		/*通知sm所选qam不可用*/
	}
	/*erm requst teardown to sm: step 2*/
	TEARDOWN_MSG2 erm2sm;
	//memset(&erm2sm,0x00,sizeof(erm2sm));
	erm2sm.rtsp_url = sm2erm.rtsp_url;
	erm2sm.cseq = sm2erm.cseq;
	erm2sm.reason = sm2erm.reason;
	erm2sm.session = sm2erm.session;
	erm2sm.ondemandsessionid = sm2erm.ondemandsessionid;

	rtsp_s6_teardown_msg_encode(erm2sm, sendbuf);

	//memset(sendbuf,0x00,sizeof(sendbuf));
	//memset(buf,0x00,sizeof(buf));
	log(LVLDEBUG,SYS_INFO,"erm2SM teardown(step 2) msg:%s, len:%d\n",sendbuf,strlen(sendbuf));

	rtsp_write(SM_sd, sendbuf, strlen(sendbuf));
	ret = rtsp_read(SM_sd, buf, 1024);

	log(LVLDEBUG,SYS_INFO,"SM2erm teardown(step 3) response(2) msg:%s, len:%d\n",buf,ret);

	/* sm response step 2 teardown*/
	TEARDOWN_RES1 sm_resp2erm;
	str = buf;

	rtsp_s6_teardown_res_parse(str, &sm_resp2erm);

	qam_static.udp_state = 3;
	/*teardown pre3*/
	//lock
	ret = QAM_SETUP_DOWN(qam_static);
	//unlock
	log(LVLDEBUG,SYS_INFO,"QAM_SETUP_DOWN return %d\n",ret);
	if (ret) {
	}
	/*to qam*/
//tqam 有问题 没有初始化
	str = rtsp_r6_teardown_msg_encode(qam_addr, tqam, erm_cseq,
			atoi(sm2erm.reason), sm2erm.session, sm2erm.ondemandsessionid);
	//memset(sendbuf,0x00,sizeof(sendbuf));
	//memset(buf,0x00,sizeof(buf));
	memcpy(sendbuf, str.c_str(), str.length());
	log(LVLDEBUG,SYS_INFO,"erm2qam teardown(step 4)  msg:%s, len:%d\n",sendbuf,str.length());

	rtsp_write(qam_sd, sendbuf, str.length());
	ret = rtsp_read(qam_sd, buf, 1024);

	log(LVLDEBUG,SYS_INFO,"qam2erm teardown(step 5) response(4) msg:%s, len:%d\n",buf,ret);

	str = buf;
	if (!GetResponses(str, &err_code, &qam_cseq, &qam_session_id,
			&qam_ondemandsessionid)) {
		log(LVLDEBUG,SYS_INFO,"Qam response abnormity\n");

		resp_s6_msg.error_code = err_code;
		resp_s6_msg.session = sm2erm.session;
		resp_s6_msg.onDemandSessionId = sm2erm.ondemandsessionid;
		resp_s6_msg.cseq = sm_resp2erm.cseq;

		rtsp_s6_setup_response_encode(resp_s6_msg, sendbuf);
		rtsp_write(SM_sd, sendbuf, strlen(sendbuf));
		ret = rtsp_read(SM_sd, buf, 1024);
		return -1;
		/*
		 ann.cseq=sm_resp2erm.cseq;
		 strbuf=rtsp_s6_teardown_res_encode(ann);
		 memcpy(sendbuf,strbuf.c_str(),strbuf.length());
		 rtsp_write(SM_sd,sendbuf,strbuf.length());
		 ret=rtsp_read(SM_sd,buf,1024);
		 return 0;
		 */
		/*通知sm所选qam不可用*/
	}

	qam_static.udp_state = 4;
	/*teardown pre4*/
	//lock
	QAM_SETUP_DOWN(qam_static);
	//unlock

	/*erm response step 1 to sm*/
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

	log(LVLDEBUG,SYS_INFO,"read over\n");
	close(qam_sd);
	return 0;
}
