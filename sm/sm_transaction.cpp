/*
�汾:v1.0

 * ����:������

 * ����ʱ��:2011-05-16

 *�޸ļ�¼:

 *����˵��:
	S1 S3 S6 ҵ����

 *�������: ��

 *�������: ��

 *����ֵ:��
*/
#include "sm_rtsp_s1_msg_process.h"
#include "sm_rtsp_s3_msg_process.h"
#include "sm_rtsp_s6_msg_process.h"
#include "sm_rtsp_public_function.h"
#include "sm_communication_module.h"
#include "sm_transaction.h"
#include "sm_log.h"
#include <iostream>
#include <occi.h>
using namespace std;
using namespace oracle::occi;
extern int LVLDEBUG;

int pthread_TP(char *S1_msg,int msg_len,int OC_sd,pthread_args *p_args)
{
    int ret = -1;
    ret = rtsp_get_msg_type(S1_msg);

    if (ret == RTSP_ID_S1_SETUP) {
	fprintf(stderr, "Tp_setup begin...\n");
        Tp_Setup(S1_msg,msg_len,OC_sd,p_args);
    } else if (ret == RTSP_ID_S1_TEARDOWN) {
        Tp_Teardown(S1_msg,msg_len,OC_sd,p_args);
    } else {
        write(OC_sd,"TYPE ERROR",20);
    }
    return 0;
}

int Tp_Setup(char * S1_msg,int msg_len,int OC_sd,pthread_args *p_args)
{
    S1_SETUP_MSG s1_setup_msg;
    S1_SETUP_RES s1_setup_res;
    S3_SETUP_MSG s3_setup_msg;
    S3_SETUP_RES s3_setup_res;
    S6_SETUP_MSG s6_setup_msg;
    S6_SETUP_RES s6_setup_res;

    int erm_sd = -1;
    int odrm_sd = -1;
    INT64 oc2sm_session = 0;//stb��sm֮���session id
    int sm2erm_cseq = 0;//sm��erm֮���cseq
    int sm2odrm_cseq = 0;//sm��odrm֮���cseq
    struct timeval now_tmp;
    int i = 0;
    int ret = -1;
    char ondemandsession[37] = "";
    char sendbuf[1024] = "";//�洢������Ϣ�Ļ���
    char recvbuf[1024] = "";//�洢�յ���Ϣ�Ļ���
    char sql_cmd[512] = "";//�洢sql����
    string cmd;
//�������ݿ�
    Connection *conn = p_args->connPool->getAnyTaggedConnection();
	Statement *stmt = conn->createStatement();
	
	ondemandsessionid_generate(ondemandsession);//����ondemandsessionid
	
	gettimeofday(&now_tmp, 0);
	srand((now_tmp.tv_sec * 1000) + (now_tmp.tv_usec / 1000));
	oc2sm_session = 1 + (int) (10.0 * rand() / (100000 + 1.0));//����S1�ӿ�session id
	sm2erm_cseq = 1 + (int) (1.0 * rand() / (1000000 + 1.0));//����sm��erm֮���cseq
	sm2odrm_cseq = 1 + (int) (1.0 * rand() / (1000000 + 1.0));//����sm��odrm֮���cseq
//cout << ondemandsession	<< endl;
    
//�����յ���s1_setup_msg
	memset(&s1_setup_msg,0x00,sizeof(S1_SETUP_MSG));
	fprintf(stderr, "parse s1 message..\n");
    rtsp_s1_setup_msg_parse(S1_msg,&s1_setup_msg);//�����յ���s1_setup_msg
	/*
		��stb������setup msg���д�����
	*/    

//��ERM�����Ự
    memset(&s6_setup_msg,0x00,sizeof(S6_SETUP_MSG));
    strcpy(s6_setup_msg.ip,ERM_IP);
    s6_setup_msg.cseq = sm2erm_cseq;
    strcpy(s6_setup_msg.require,RTSP_S6_REQUIRE);
    strcpy(s6_setup_msg.session_group,SESSION_GRUOP);//�Լ��趨����Ҫ����getpramaterʱ��ȡsession�����Ϣ��ÿ��SMһ����
    strcpy(s6_setup_msg.encryption_type,"Session");//S2
    strcpy(s6_setup_msg.cas_id,"0x1234");//S2
    strcpy(s6_setup_msg.encrypt_control,"block_stream_until_encrypted=\"false\";encryption_scheme=\"AES\";key_length=128");//S2
    strcpy(s6_setup_msg.ondemandsessionid,ondemandsession);
    strcpy(s6_setup_msg.policy,"priority=1");//S2
    strcpy(s6_setup_msg.inband_marker,"type=4;pidType=A;pidValue=01EE;dataType=T;insertDuration=10000;data=4002003030��");//���������ִ�S2��ã�data��������
    s6_setup_msg.qam_num = s1_setup_msg.qam_num;
    for (i=0;i<s6_setup_msg.qam_num;i++) {
        s6_setup_msg.qam_info[i].bandwidth = 2700000;
        strcpy(s6_setup_msg.qam_info[i].qam_name,s1_setup_msg.qam[i].qam_name);
        strcpy(s6_setup_msg.qam_info[i].client,s1_setup_msg.qam[i].client);
        strcpy(s6_setup_msg.qam_info[i].modulation,"qam64");
    }
	fprintf(stderr, "connect to erm...\n");
    ret=ConnectSock(&erm_sd,ERM_PORT,ERM_IP);//������ERM������
    if (ret==-1) {
	fprintf(stderr, "connect to erm error...\n");
        sm_log(LVLDEBUG,SYS_INFO,"Connect erm error\n");
        /*
        	������
        */
        memset(sendbuf,0x00,1024);
        rtsp_err_res_encode(RTSP_ResponseCode_ERMSetupFailed_NoResponse,s1_setup_msg.cseq,sendbuf);//��STB����������ERMʧ��
        ret = rtsp_write(OC_sd,sendbuf,strlen(sendbuf)+1);//��STB����ERROR��Ϣ
    	sm_log(LVLDEBUG,SYS_INFO,"setup send2oc msg:%s, len:%d\n",sendbuf,ret);        
        return -1;
    }

	fprintf(stderr, "connect to erm success...\n");
    memset(sendbuf,0x00,1024);		
	fprintf(stderr, "msg to S6 encode\n");
    rtsp_s6_setup_msg_encode(s6_setup_msg,sendbuf);//����ģʽ�»򱯹�ģʽ�´���SM����ERM��SETUP��Ϣ
	fprintf(stderr, "send msg to S6\n");
    ret = rtsp_write(erm_sd,sendbuf,strlen(sendbuf)+1);//��ERM����SETUP��Ϣ
    sm_log(LVLDEBUG,SYS_INFO,"setup send2erm msg:%s, len:%d\n",sendbuf,ret);
//����ERM���ص�SETUP RESPONSE
	memset(recvbuf,0x00,1024);    
    ret = rtsp_read(erm_sd,recvbuf,1024) ;//����ERM���ص�SETUP RESPONSE
	fprintf(stderr, "read msg from S6 done\n");
    sm_log(LVLDEBUG,SYS_INFO,"setup recv from erm res:%s, len:%d\n",recvbuf,ret);
    //����ģʽ�»򱯹�ģʽ�½���ERM��SM���͵�Response��Ϣ
	memset(&s6_setup_res,0x00,sizeof(S6_SETUP_RES));
	rtsp_s6_setup_res_parse(recvbuf,&s6_setup_res);
	fprintf(stderr, "parse msg from S6 done\n");
	/*
		��erm������setup response���д�����
	*/
	//����ERM�ĻỰ��Ϣ���
	snprintf(sql_cmd,512,"INSERT INTO SM_S6 VALUES('%s',%llu,'%s','%s','%s','%s','%s','%s','%s','%s',%llu,'%s','%s',%d,'%s','%s','%s','%s','%s','%s')", \
						 ondemandsession,s6_setup_res.session,s6_setup_msg.ip,s6_setup_msg.session_group,s6_setup_msg.encryption_type,\
						 s6_setup_msg.cas_id,s6_setup_msg.encrypt_control,s6_setup_msg.policy,s6_setup_msg.inband_marker,s6_setup_res.embedded_encryptor,\
						 s6_setup_msg.qam_info[0].bandwidth,s6_setup_res.client,s6_setup_res.destination,s6_setup_res.client_port,\
						 s6_setup_res.qam_destination,s6_setup_res.qam_name,s6_setup_res.qam_group,s6_setup_res.modulation,s6_setup_res.edge_input_group,"SETUP");    
	cmd = sql_cmd;
	stmt->executeUpdate(cmd.c_str());
	stmt->executeUpdate("commit");
	
//��ODRM�����Ự
	memset(&s3_setup_msg,0x00,sizeof(S3_SETUP_MSG));
    strcpy(s3_setup_msg.odrm_ip,"ODRM_IP");
    s3_setup_msg.odrm_port = ODRM_PORT;
    s3_setup_msg.cseq = sm2odrm_cseq;
    strcpy(s3_setup_msg.require,RTSP_S3_REQUIRE);
    strcpy(s3_setup_msg.ondemand_session_id,ondemandsession);

    strcpy(s3_setup_msg.sop_group[0],"boston.spg1");//����
    strcpy(s3_setup_msg.sop_group[1],"boston.spg2");//����

    s3_setup_msg.qam_num = 1;//����ģʽ��ERM����ΨһQAM
    strcpy(s3_setup_msg.qam[0].client,s6_setup_res.client);
    strcpy(s3_setup_msg.qam[0].destination,s6_setup_res.destination);
    s3_setup_msg.qam[0].client_port = s6_setup_res.client_port;
    s3_setup_msg.qam[0].bandwidth = 2700000; //��S2��Ϣ��Resource Descriptors�е�TSDownstreamBandwidth���
    	
    strcpy(s3_setup_msg.session_group,SESSION_GRUOP);
    s3_setup_msg.start_point_slot = 1;//S2���
    strcpy(s3_setup_msg.start_point_npt,"3000");//S2���
    strcpy(s3_setup_msg.policy,"priority=1");//S2���
    strcpy(s3_setup_msg.inband_marker,"type=4;pidType=A;pidValue=01EE;dataType=T;insertDuration=10000;data=4002003030");//���������ִ�S2��ã�data��������

    s3_setup_msg.sdp_version = 0;
    strcpy(s3_setup_msg.email_add,"-");
    
    sprintf(s3_setup_msg.ntp,"%lu",NTP_time(time(NULL)));//���ɻỰNTPʱ��
    strcpy(s3_setup_msg.add_type,"IN");
    strcpy(s3_setup_msg.ip_version,"IP4");
    strcpy(s3_setup_msg.sm_ip,SM_IP);
    strcpy(s3_setup_msg.s," ");
    s3_setup_msg.time[0] = 0;//�Ự��ʼʱ��
    s3_setup_msg.time[1] = 0;//�Ự����ʱ�� 0 0��ʾ��Զ����
    strcpy(s3_setup_msg.provider_id,"comcast.com");//S2
    strcpy(s3_setup_msg.asset_id,"abcd1234567890123456");//S2
    s3_setup_msg.start_npt = 1000;//S2
    s3_setup_msg.stop_npt = 6000;//S2

    strcpy(s3_setup_msg.c,"IN IP4 0.0.0.0");
    strcpy(s3_setup_msg.m,"video 0 udp MP2T");

    memset(sendbuf,0x00,1024);
    rtsp_s3_setup_msg_encode(s3_setup_msg,sendbuf);
    // printf("%s",sendbuf);

    ret=ConnectSock(&odrm_sd,ODRM_PORT,ODRM_IP);
    if (ret==-1) {
        sm_log(LVLDEBUG,SYS_INFO,"Connect odrm error\n");
        /*
        	������
        */
        memset(sendbuf,0x00,1024);
        rtsp_err_res_encode(RTSP_ResponseCode_ODRMSetupFailed_NoResponse,s1_setup_msg.cseq,sendbuf);//��STB����������ODRMʧ��
        ret = rtsp_write(OC_sd,sendbuf,strlen(sendbuf)+1);//��STB����ERROR��Ϣ
    	sm_log(LVLDEBUG,SYS_INFO,"setup send2oc msg:%s, len:%d\n",sendbuf,ret);   
        return -1;
    }
    ret = rtsp_write(odrm_sd,sendbuf,strlen(sendbuf)+1);//��ODRM����SETUP��Ϣ
    sm_log(LVLDEBUG,SYS_INFO,"setup send2odrm msg:%s, len:%d\n",sendbuf,ret);
//����ODRM���ص�SETUP RESPONSE
    memset(recvbuf,0x00,1024);
    ret = rtsp_read(odrm_sd,recvbuf,1024);//����ODRM���ص�SETUP RESPONSE  
    sm_log(LVLDEBUG,SYS_INFO,"setup recv from odrm res:%s, len:%d\n",recvbuf,ret);
    memset(&s3_setup_res,0x00,sizeof(S3_SETUP_RES));
    rtsp_s3_setup_res_parse(recvbuf,&s3_setup_res);//����ODRM����SM��SETUP RESPONSE��Ϣ 
	/*
		��odrm������setup response���д�����
	*/
	//����ODRM�ĻỰ��Ϣ���
	//snprintf(sql_cmd,512,"INSERT INTO SM_S6 VALUES('%s',%llu,'%s','%s','%s','%s','%s','%s','%s','%s',%llu,'%s','%s',%d,'%s','%s','%s','%s','%s','%s')", \
	//					 ondemandsession,s6_setup_res.session,s6_setup_msg.ip,s6_setup_msg.session_group,s6_setup_msg.encryption_type,\
	//					 s6_setup_msg.cas_id,s6_setup_msg.encrypt_control,s6_setup_msg.policy,s6_setup_msg.inband_marker,s6_setup_res.embedded_encryptor,\
	//					 s6_setup_msg.qam_info[0].bandwidth,s6_setup_res.client,s6_setup_res.destination,s6_setup_res.client_port,\
	//					 s6_setup_res.qam_destination,s6_setup_res.qam_name,s6_setup_res.qam_group,s6_setup_res.modulation,s6_setup_res.edge_input_group,"SETUP");    
	memset(sql_cmd,0x00,sizeof(sql_cmd));
	snprintf(sql_cmd,512,"INSERT INTO SM_S3 VALUES('%s',%llu,'%s',%d,'%s','%s','%s',%d,'%s','%s','%s','%s','%s',%d,'%s',%d,%llu,'%s',%d,%d,'%s','%s',%d,%d,%llu,'%s','%s','%s','%s',%d,%llu,'%s')",\
						 ondemandsession,s3_setup_res.session,s3_setup_msg.odrm_ip,s3_setup_msg.odrm_port,s3_setup_res.sop_group,s3_setup_res.sop,\
						 s3_setup_msg.session_group,s3_setup_msg.start_point_slot,s3_setup_msg.start_point_npt,s3_setup_msg.policy,s3_setup_msg.inband_marker,\
						 s3_setup_res.client,s3_setup_res.destination,s3_setup_res.client_port,s3_setup_res.source,s3_setup_res.server_port,s3_setup_res.bandwidth,\
						 s3_setup_msg.sm_ip,s3_setup_msg.time[0],s3_setup_msg.time[1],s3_setup_msg.provider_id,s3_setup_msg.asset_id,s3_setup_msg.start_npt,s3_setup_msg.stop_npt,\
						 s3_setup_res.ss_session,s3_setup_res.ntp,s3_setup_res.ss_ip,s3_setup_res.protocol,s3_setup_res.host,s3_setup_res.port,s3_setup_res.stream_handle,"SETUP");    
	cmd = sql_cmd;
	stmt->executeUpdate(cmd.c_str());
	stmt->executeUpdate("commit");
    

//��STB����setup response��Ϣ
    //�������session��
	memset(&s1_setup_res,0x00,sizeof(S1_SETUP_RES));	
    s1_setup_res.cseq = s1_setup_msg.cseq;
    s1_setup_res.session = oc2sm_session;
    strcpy(s1_setup_res.destination,s6_setup_res.qam_destination);
    strcpy(s1_setup_res.ondemand_session_id,ondemandsession);
    strcpy(s1_setup_res.client_session_id,s1_setup_msg.client_session_id);
    strcpy(s1_setup_res.emm_data,"40203F21A5");//����
    s1_setup_res.sdp_version = 0;
    strcpy(s1_setup_res.email_add,"-");
    s1_setup_res.ss_session = s3_setup_res.ss_session;
    strcpy(s1_setup_res.ntp,s3_setup_res.ntp);
    strcpy(s1_setup_res.add_type,"IN");
    strcpy(s1_setup_res.ip_version,"IP4");
    strcpy(s1_setup_res.ss_ip,s3_setup_res.ss_ip);
    strcpy(s1_setup_res.s," ");
    s1_setup_res.time[0] = s3_setup_res.time[0];
    s1_setup_res.time[1] = s3_setup_res.time[1];
    strcpy(s1_setup_res.protocol,s3_setup_res.protocol);
    strcpy(s1_setup_res.host,s3_setup_res.host);
    s1_setup_res.port = s3_setup_res.port;
    s1_setup_res.stream_handle = s3_setup_res.stream_handle;
    strcpy(s1_setup_res.c,"IN IP4 0.0.0.0");
    strcpy(s1_setup_res.m,"video 0 udp MP2T");

    memset(sendbuf,0x00,1024);
    rtsp_s1_setup_res_encode(s1_setup_res,sendbuf);//����SM����STB��setup response��Ϣ
    ret = rtsp_write(OC_sd,sendbuf,strlen(sendbuf)+1);//��STB����SETUP��Ϣ
    sm_log(LVLDEBUG,SYS_INFO,"setup send2oc msg:%s, len:%d\n",sendbuf,ret);
    	
    //����STB�ĻỰ��Ϣ���
    snprintf(sql_cmd,512,"INSERT INTO SM_S1 VALUES('%s','%s',%llu,'%s',%d,'%s','%s','%s','%s',%llu,'%s','%s',%d,%d,'%s','%s',%d,%llu,'%s')",\
    					 ondemandsession,s1_setup_res.client_session_id,s1_setup_res.session,s1_setup_msg.sm_ip,s1_setup_msg.sm_port,\
    					 s1_setup_msg.purchase_token,s1_setup_msg.server_id,s1_setup_res.emm_data,s1_setup_res.destination,\
    					 s1_setup_res.ss_session,s1_setup_res.ntp,s1_setup_res.ss_ip,s1_setup_res.time[0],s1_setup_res.time[1],\
    					 s1_setup_res.protocol,s1_setup_res.host,s1_setup_res.port,s1_setup_res.stream_handle,"SETUP");    
	cmd = sql_cmd;
	stmt->executeUpdate(cmd.c_str());
    stmt->executeUpdate("commit");

//�ر����ݿ�����
	conn->terminateStatement(stmt);
    p_args->connPool->releaseConnection(conn);
    return 0;
}

int Tp_Teardown(char * S1_msg,int msg_len,int OC_sd,pthread_args *p_args)
{
    S1_TEARDOWN_MSG s1_tear_msg;
    S1_TEARDOWN_RES s1_tear_res;
    S3_TEARDOWN_MSG s3_tear_msg;
    S3_TEARDOWN_RES s3_tear_res;
    S6_TEARDOWN_MSG1 s6_tear_msg1;
    S6_TEARDOWN_RES2 s6_tear_res2;

    int erm_sd = -1;
    int odrm_sd = -1;
    int ret = -1;
    char sendbuf[1024] = "";
    char recvbuf[1024] = "";
    char sql_cmd[256] = "";//�洢sql����
    string cmd;
    int oc2sm_session = 0;
	int odrm2sm_session = 0;//odrm��sm֮���session id
	int erm2sm_session = 0;//erm��sm֮���session id
	int sm2erm_cseq = 0;
	int sm2odrm_cseq = 0;
	struct timeval now_tmp;
	
//�������ݿ�   
    Connection *conn = p_args->connPool->getAnyTaggedConnection();
	Statement *stmt = conn->createStatement();

//����session��cseq	
	gettimeofday(&now_tmp, 0);
	srand((now_tmp.tv_sec * 1000) + (now_tmp.tv_usec / 1000));
	oc2sm_session = 1 + (int) (10.0 * rand() / (100000 + 1.0));//����S1�ӿ�session id
	sm2erm_cseq = 1 + (int) (1.0 * rand() / (1000000 + 1.0));//����sm��erm֮���cseq
	sm2odrm_cseq = 1 + (int) (1.0 * rand() / (1000000 + 1.0));//����sm��odrm֮���cseq

//����STB����SM��TEARDOWN��Ϣ
    rtsp_s1_teardown_msg_parse(S1_msg,&s1_tear_msg);//����STB����SM��TEARDOWN��Ϣ


	
//������ODRM�ĻỰ
	//���ݿ��ѯָ��ҵ��ondemandsessionid��S3�ӿڵĻỰ��    
    snprintf(sql_cmd,256,"SELECT SESSION_ID FROM SM_S3 WHERE ONDEMAND_SESSION_ID='%s'",s1_tear_msg.ondemand_session_id);
    cmd = sql_cmd;
    ResultSet *rs = stmt->executeQuery(cmd.c_str());
    rs->next();
    odrm2sm_session = (int)rs->getNumber(1);
    //cout << odrm2sm_session << endl;
    
    strcpy(s3_tear_msg.odrm_ip,ODRM_IP);
    s3_tear_msg.odrm_port = ODRM_PORT;
    s3_tear_msg.cseq = sm2odrm_cseq;
    s3_tear_msg.reason = s1_tear_msg.reason;
    s3_tear_msg.session = odrm2sm_session;
    strcpy(s3_tear_msg.ondemand_session_id,s1_tear_msg.ondemand_session_id);

    rtsp_s3_teardown_msg_encode(s3_tear_msg,sendbuf);

    ret=ConnectSock(&odrm_sd,ODRM_PORT,ODRM_IP);
    if (ret==-1) {
        sm_log(LVLDEBUG,SYS_INFO,"Connect odrm error");
        /*
        	������
        */
        return -1;
    }
    ret = rtsp_write(odrm_sd,sendbuf,strlen(sendbuf)+1);//��ODRM����teardown��Ϣ
    sm_log(LVLDEBUG,SYS_INFO,"teardown send2odrm msg:%s, len:%d\n",sendbuf,ret);
    ret = rtsp_read(odrm_sd,recvbuf,1024);//����ODRM���ص�teardown response
    sm_log(LVLDEBUG,SYS_INFO,"teardown recv from odrm res:%s, len:%d\n",recvbuf,ret);
    //����odrm���� ��teardown response
    rtsp_s3_teardown_res_parse(recvbuf,&s3_tear_res);
	//�޸����ݿ�״̬
	snprintf(sql_cmd,256,"UPDATE SM_S3 SET STATUS='TEARDOWN' WHERE ONDEMAND_SESSION_ID='%s'",s1_tear_msg.ondemand_session_id);    
	cmd = sql_cmd;
	stmt->executeUpdate(cmd.c_str());
    stmt->executeUpdate("commit");    


    
//������ERM�ĻỰ������ģʽ��
	//���ݿ��ѯָ��ҵ��ondemandsessionid��S3�ӿڵĻỰ��    
    snprintf(sql_cmd,256,"SELECT SESSION_ID FROM SM_S6 WHERE ONDEMAND_SESSION_ID='%s'",s1_tear_msg.ondemand_session_id);
    cmd = sql_cmd;
    rs = stmt->executeQuery(cmd.c_str());
    rs->next();
    erm2sm_session = (int)rs->getNumber(1);
    
    strcpy(s6_tear_msg1.rtsp_ip,ERM_IP);
    s6_tear_msg1.cseq = sm2erm_cseq;
    s6_tear_msg1.reason = s1_tear_msg.reason;
    s6_tear_msg1.session = erm2sm_session;
    strcpy(s6_tear_msg1.ondemandsessionid,s1_tear_msg.ondemand_session_id);
    
    memset(sendbuf,0x00,1024);
    rtsp_s6_teardown_msg_encode(s6_tear_msg1,sendbuf);//������STB���͵�teardown response
    
    ret=ConnectSock(&erm_sd,ERM_PORT,ERM_IP);
    if (ret==-1) {
        sm_log(LVLDEBUG,SYS_INFO,"Connect erm error");
        /*
        	������
        */
        return -1;
    }
    ret = rtsp_write(erm_sd,sendbuf,strlen(sendbuf)+1);//��ERM����SETUP��Ϣ
    sm_log(LVLDEBUG,SYS_INFO,"teardown send2erm msg:%s, len:%d\n",sendbuf,ret);
    memset(recvbuf,0x00,1024);
    ret = rtsp_read(erm_sd,recvbuf,1024);//����ERM���ص�SETUP RESPONSE
    sm_log(LVLDEBUG,SYS_INFO,"teardown recv from erm res:%s, len:%d\n",recvbuf,ret);
    //����ERM����SM��Teardown Response
	rtsp_s6_teardown_res_parse(recvbuf,&s6_tear_res2);
	   
    //�޸����ݿ�״̬
	snprintf(sql_cmd,256,"UPDATE SM_S6 SET STATUS='TEARDOWN' WHERE ONDEMAND_SESSION_ID='%s'",s1_tear_msg.ondemand_session_id);    
	cmd = sql_cmd;
	stmt->executeUpdate(cmd.c_str());
    stmt->executeUpdate("commit"); 

//��STB����teardown response
    s1_tear_res.err_code = 200;//�Լ����ã�ע���������
    s1_tear_res.cseq = s1_tear_msg.cseq;
    s1_tear_res.session = oc2sm_session;
    strcpy(s1_tear_res.ondemand_session_id,s1_tear_msg.ondemand_session_id);
    strcpy(s1_tear_res.client_session_id,s1_tear_msg.client_session_id);
	memset(sendbuf,0x00,1024);
    rtsp_s1_teardown_res_encode(s1_tear_res,sendbuf);
    ret = rtsp_write(OC_sd,sendbuf,strlen(sendbuf)+1);//��STB����teardown response��Ϣ
    sm_log(LVLDEBUG,SYS_INFO,"teardown send2oc res:%s, len:%d\n",sendbuf,ret);
    //�޸����ݿ�״̬
	snprintf(sql_cmd,256,"UPDATE SM_S1 SET STATUS='TEARDOWN' WHERE ONDEMAND_SESSION_ID='%s'",s1_tear_msg.ondemand_session_id);    
	cmd = sql_cmd;
	stmt->executeUpdate(cmd.c_str());
    stmt->executeUpdate("commit"); 

//�ر����ݿ�����
	stmt->closeResultSet(rs);
	conn->terminateStatement(stmt);
    p_args->connPool->releaseConnection(conn);
    return 0;
}