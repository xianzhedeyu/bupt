#include "ermi.h"


int main()
{
/*

    string str = "SETUP rtsp://192.26.2.2 RTSP/1.0\r\n"
"CSeq: 314\r\n"
"Require: ermi\r\n"
"Transport: DOCSIS/QAM;unicast;bit_rate=38000000;qam_id=456;depi_mode=docsis_mpt,DOCSIS/QAM;unicast;bit_rate=38000000;qam_id=123;depi_mode=docsis_mpt\r\n\r\n";

   SETUP_MSG setup_msg;
    rtsp_ermi_setup_parse(str,&setup_msg);
    cout << setup_msg.rtsp_url<<endl;
    cout << setup_msg.cseq<<endl;
    cout << setup_msg.require<<endl;   
    cout << setup_msg.qaminf[0].client << endl;
    cout << setup_msg.qaminf[0].bit_rate << endl;
    cout << setup_msg.qaminf[0].qam_id << endl;
    cout << setup_msg.qaminf[0].depi_mode << endl;

    cout << setup_msg.qaminf[1].client << endl;
    cout << setup_msg.qaminf[1].bit_rate << endl;
    cout << setup_msg.qaminf[1].qam_id << endl;
    cout << setup_msg.qaminf[1].depi_mode << endl;

  */
 
/*
  // 创建ERM发给SM的setup response消息
	SETUP_RESPONSE res;
	res.error_code = 200;
	res.cseq = 123;
	res.session = 1111111;
	strcpy(res.client,"00AF123456DE");
	strcpy(res.qam_destination,"550000000.15");
	strcpy(res.destination,"192.26.13.1");
	res.client_port = 4588;
	strcpy(res.qam_name,"Chicago.SouthBend.5");
	strcpy(res.qam_group,"Chicago.SouthBend.QG001");
	strcpy(res.modulation,"qam64");
	strcpy(res.edge_input_group,"Chicago.SouthBend.S101");
	strcpy(res.embeddedEncryptor,"Yes");
	strcpy(res.onDemandSessionId,"be074250-cc5a-11d9-8cd5-0800200c9a66");
	
	char a[600];
	rtsp_s6_setup_response_encode(res,a);
	cout << a;
*/
	
/*
//解析SM发给ERM的teardown response消息	
	string str = "TEARDOWN rtsp://srm1.net RTSP/1.0\r\n"
                 "CSeq: 314\r\n"
                 "Require: com.comcast.ngod.s6\r\n"
				 "Reason: 200 \"User stop\"\r\n"
				 "Session: 47112344\r\n"
				 "OnDemandSessionId: be074250-cc5a-11d9-8cd5-0800200c9a66\r\n\r\n";
	TEARDOWN_MSG1 a;
	rtsp_s6_teardown_msg_parse(str,&a);
    cout<<a.rtsp_url<<endl;
    cout<<a.cseq<<endl;
    cout<<a.require<<endl;
    cout<<a.reason<<endl;
    cout<<a.session<<endl;
    cout<<a.ondemandsessionid<<endl;
 */
 /* 
  //创建ERM给SM发的teardown消息
    TEARDOWN_MSG2 tear;
    strcpy(tear.rtsp_url,"rtsp://srm1.net");
    tear.cseq=1;
    strcpy(tear.reason,"200 \"User stop\"");
    tear.session=333;
    strcpy(tear.ondemandsessionid,"be074250-cc5a-11d9-8cd5-0800200c9a66");

	char  b[600];
     rtsp_s6_teardown_msg_encode(tear,b);
    cout << b;
   */
   
  /* 
//解析SM发给ERM的teardown response消息
    string str = "RTSP/1.0 200 OK\r\nCSeq: 414\r\nSession: 3500123\r\nOnDemandSessionId: be074250-cc5a-11d9-8cd5-0800200c9a66\r\n\r\n";
    
    TEARDOWN_RES1 r;
    rtsp_s6_teardown_res_parse(str,&r);
    cout << r.cseq<<endl;
    cout << r.session<<endl;
    cout << r.ondemandsessionid<<endl;
  */  
   /*
    //创建ERM给SM发的teardown response消息
    TEARDOWN_RES2 tear;
    tear.error_code = 200;
    tear.cseq=9;
    tear.session=32198;
    strcpy(tear.ondemandsessionid,"be074250-cc5a-11d9-8cd5-0800200c9a66");
    char resp_str[300];
    rtsp_s6_teardown_res_encode(tear,resp_str);
    cout<< resp_str;
   */ 
    
  /*  
    //创建ERM给SM发的announce消息
    ANNOUNCE_MSG ann;
   // ann.rtspurl = "rtsp://srm1.net";
    strcpy(ann.rtspurl,"rtsp://srm1.net");
    ann.cseq = 3;
    ann.session = 4533432;
    //ann.notice = "5401 \"Downstream Failure\"";
    strcpy(ann.notice,"5401 \"Downstream Failure\"");
    //ann.event_date = "19930316T064707.735Z";
    strcpy(ann.event_date,"19930316T064707.735Z");
    //ann.npt = "";
    strcpy(ann.npt,"");
    //ann.ondemandsessionid="be074250-cc5a-11d9-8cd5-0800200c9a66";
    strcpy(ann.ondemandsessionid,"be074250-cc5a-11d9-8cd5-0800200c9a66");
    char a[400];
    rtsp_s6_announce_res_encode(ann,a);
    cout << a;
    */
   
   /*
    //创建ERM发给SM的错误response消息
   cout << rtsp_s6_error_res_encode(400,343);
   */
   
   //获取消息类型
//   string str = "ANE FDSF";
//   cout << rtsp_s6__response_parse(str) <<endl;
   
   /*SETUP_MSG1 msg1;
   msg1.rtsp_ip = "192.168.1.1";
   msg1.cseq = 123;
   msg1.session_group = "Chicago.SouthBend.SM1";
   msg1.ondemandsessionid = "be074250-cc5a-11d9-8cd5-0800200c9a66";
   msg1.policy = "priority=1";
   msg1.qam_info1[0].destination = "192.26.13.1";
   msg1.qam_info1[0].client_port = "4588";
   msg1.qam_info1[0].client = "00AF123456DE";
      msg1.qam_info1[1].destination = "192.26.13.1";
   msg1.qam_info1[1].client_port = "4588";
   msg1.qam_info1[1].client = "00AF123456DE";
         msg1.qam_info1[2].destination = "192.26.13.1";
   msg1.qam_info1[2].client_port = "4588";
   msg1.qam_info1[2].client = "00AF123456DE";
char resp_str[500];
  rtsp_s6_setup_msg_encode(msg1,resp_str);
  cout << resp_str;	*/
 /* SETUP_RESPONSE1 resp_msg;
  string str = "RTSP/1.0 200 OK\r\nCSeq: 413\r\nSession: 3500123\r\nTransport: MP2T/DVBC/UDP;unicast;destination=192.26.14.1;client_port=4000;source=192.26.18.1;server_port=625;client=00AF123456DE\r\nEmbeddedEncryptor: Yes\r\nOnDemandSessionId: be074250-cc5a-11d9-8cd5-0800200c9a66\r\n\r\n";
  rtsp_s6_setup_response_parse(str, &resp_msg);
  cout << resp_msg.error_code<<endl;
  cout << resp_msg.cseq<<endl;
    cout << resp_msg.session<<endl;
      cout << resp_msg.destination<<endl;
        cout << resp_msg.client_port<<endl;
          cout << resp_msg.source<<endl;
            cout << resp_msg.server_port<<endl;
              cout << resp_msg.client<<endl;
                cout << resp_msg.embeddedEncryptor<<endl;
                  cout << resp_msg.onDemandSessionId<<endl;*/

int a=9;
char tbuf[1024];
sprintf(tbuf,"%d",a);
printf("%s",tbuf);                  
    return 0;  
}
