#include "sm_rtsp_s1_msg_process.h"
#include "sm_rtsp_public_function.h"

//解析STB发给SM的SETUP消息
int rtsp_s1_setup_msg_parse(char *setup_msg,S1_SETUP_MSG *msg)
{
	char str[1024] = "";
	strncpy(str,setup_msg,strlen(setup_msg) - 4);//复制setup消息并去除最后的"\r\n\r\n"
	
	//printf("%s\n",str);
	char *line = NULL;
	char *subline = NULL;
	char *header = NULL;
	char *header_val = NULL;
	char *subheader = NULL;
	char *subheader_val = NULL;
	char *ptr = NULL;
	char *ptr1 = NULL;
	char *ptr2 = NULL;
	char *ptr3 = NULL;
	char *ptr4 = NULL;
	char *ptr5 = NULL;
	char *ptr6 = NULL;
	char *ptr7 = NULL;
	char *method = NULL;
	char *url = NULL;
	int i = 0;//记录消息中qam信息个数
	
	line = strtok_r(str,"\r\n",&ptr);
	//printf("%s\n",line);
	
	//解析消息第一行
	method = strtok_r(line," ",&ptr1);
	//printf("%s\n",method);
	
	if(strcmp(RTSP_METHOD_SETUP,method) != 0)//判断消息方法是否匹配
	{
		//printf("Method is not matched!\n");
		return -1;
	}

	subline = strtok_r(NULL," ",&ptr1);
	url = strtok_r(subline,";",&ptr2);
	parse_url(url,msg->sm_ip,&(msg->sm_port),NULL);
	subheader = strtok_r(NULL,";",&ptr2);
	//printf("%s\n",url);
	
	while(subheader)
	{
		if(strtok_r(subheader,"=",&ptr3))
		{
			if(strcmp(subheader,"purchaseToken") == 0)
			{
				subheader_val = strtok_r(NULL,"=",&ptr3);
				strcpy(msg->purchase_token,subheader_val);
				//printf("purchase:%s\n",msg->purchase_token);	
			}else if(strcmp(subheader,"serverID") == 0)
			{
				subheader_val = strtok_r(NULL,"=",&ptr3);
				strcpy(msg->server_id,subheader_val);
				//printf("server_id:%s\n",msg->server_id);	
			}
		}
		subheader = strtok_r(NULL,";",&ptr2);
	}
	strcpy(msg->rtsp_version,strtok_r(NULL," ",&ptr1));
	//printf("rtsp_version:%s\n",msg->rtsp_version);
	//printf("%s %d\n",msg->sm_ip,msg->sm_port);
		
	//按行解析剩余消息
	line = strtok_r(NULL,"\r\n",&ptr);
	while(line)
	{
		if (header = strtok_r(line,":",&ptr4))
        {
            if (strcmp(header,RTSP_HEADER_CSEQ) == 0)
            {
                header_val = strtok_r(NULL,":",&ptr4);
                msg->cseq = atoi(header_val);
                //printf("cseq:%d\n",msg->cseq);
            }
            else if (strcmp(header,RTSP_HEADER_REQUIRE) == 0)
            {
                header_val = strtok_r(NULL,":",&ptr4);
                trim(header_val);
                strcpy(msg->require,header_val);
                //printf("require:%s\n",msg->require);
            }
            else if (strcmp(header,RTSP_HEADER_TRANSPORT) == 0)
            {
                header_val = strtok_r(NULL,":",&ptr4);
                //printf("header_val:%s\n",header_val);
                subline = strtok_r(header_val,",",&ptr5);
            	while(subline)
           		{
					//printf("subline:%s\n",subline);
					subheader = strtok_r(subline,";",&ptr6);
                	while (subheader != NULL)
               		{
                    	if (strtok_r(subheader,"=",&ptr7))
                    	{
                        	if (strcmp(subheader,"client") == 0)
                        	{
                            	subheader_val = strtok_r(NULL,"=",&ptr7);
                            	strcpy(msg->qam[i].client,subheader_val);
                            	//printf("%s\n",msg->qam[i].client);
                        	}else if (strcmp(subheader,"qam_name") == 0)
                        	{
                           		subheader_val = strtok_r(NULL,"=",&ptr7);
                           		trim(subheader_val);
                            	strcpy(msg->qam[i].qam_name,subheader_val);
                            	//printf("%s\n",msg->qam[i].qam_name);
                        	}                                                
                    	}
                    	subheader = strtok_r(NULL,";",&ptr6);          	
                	}
                	subline = strtok_r(NULL,",",&ptr5);
                	i++;
            	}
				msg->qam_num = i;
                //printf("%d\n",msg->qam_num);
            }else if (strcmp(header,RTSP_HEADER_CLIENTSESSIONID) == 0)
            {
                header_val = strtok_r(NULL,":",&ptr4);
                strcpy(msg->client_session_id,header_val);
                trim(msg->client_session_id);
                //printf("clientsessionid:%s\n",msg->client_session_id);
            }
        }	
		line = strtok_r(NULL,"\r\n",&ptr);
	}
	
		
	return 0;
}


//创建SM发给STB的SETUP RESPONSE消息
int rtsp_s1_setup_res_encode(S1_SETUP_RES res,char *setup_res)
{
	char sdp[1024];
	char str[2048];
	
	sprintf(sdp,"v=%d\r\n"
				"o=%s %llu %s %s %s %s\r\n"
				"s=%s\r\n"
				"t=%d %d\r\n"
				"a=control:%s://%s:%d/%llu\r\n"
				"c=%s\r\n"
				"m=%s\r\n",
			res.sdp_version,res.email_add,res.ss_session,res.ntp,res.add_type,res.ip_version,res.ss_ip,res.s,res.time[0],res.time[1],\
			res.protocol,res.host,res.port,res.stream_handle,res.c,res.m);
	sprintf(str,"RTSP/1.0 200 OK\r\n" 
				"CSeq: %d\r\n" 
				"Session: %llu\r\n"
				"Transport: MP2T/DVBC/QAM;unicast; destination=%s\r\n"
				"OnDemandSessionId: %s\r\n"
				"ClientSessionId: %s\r\n"
				"EMMData: %s\r\n"
				"Content-type: application/sdp\r\n"
				"Content-length: %d\r\n\r\n"
				"%s",
			res.cseq,res.session,res.destination,res.ondemand_session_id,res.client_session_id,res.emm_data,strlen(sdp),sdp);
	strcpy(setup_res,str);
	
	return 0;	
}
//解析STB发给SM的TEARDOWN消息
int rtsp_s1_teardown_msg_parse(char *tear_msg,S1_TEARDOWN_MSG *tear)
{
	char str[1024] = "";
	char *method = NULL;
	char *url = NULL;
	char *rtsp_version = NULL;
	char *line = NULL;
	char *header = NULL;
	char *header_value = NULL;
	char *ptr = NULL;
	char *ptr1 = NULL;
	char *ptr2 = NULL;
	
	strncpy(str,tear_msg,strlen(tear_msg)-4);//复制teardown消息（不复制消息最后的"\r\n\r\n"）
	//解析消息第一行
	line = strtok_r(str,"\r\n",&ptr);
	method = strtok_r(line," ",&ptr1);
	if(strcmp(method,RTSP_METHOD_TEARDOWN))
	{
		printf("Method is not matched!");
		return -1;
	}
	url = strtok_r(NULL," ",&ptr1);
	trim(url);
	parse_url(url,tear->sm_ip,&(tear->sm_port),NULL);//解析出消息中的sm_ip和sm_port
	rtsp_version = strtok_r(NULL," ",&ptr1);
	if(strcmp(rtsp_version,RTSP_VERSION))
	{
		printf("RTSP_VERSION is not matched!");
		return -1;	
	}
	//printf("%s %d\n",tear->sm_ip,tear->sm_port);
	//按行解析剩余消息
	line = strtok_r(NULL,"\r\n",&ptr);	
	while(line != NULL)
	{	
		header = strtok_r(line,":",&ptr2);
		if(strcmp(header,RTSP_HEADER_CSEQ) == 0)
		{
			header_value = strtok_r(NULL,":",&ptr2);
			tear->cseq = atoi(header_value);
			//printf("cseq:%d\n",tear->cseq);
		}else if(strcmp(header,RTSP_HEADER_REQUIRE) == 0)
		{
			header_value = strtok_r(NULL,":",&ptr2);
			strcpy(tear->require,header_value);
			trim(tear->require);
			//printf("require:%s\n",tear->require);
		}else if(strcmp(header,RTSP_HEADER_REASON) == 0)
		{
			header_value = strtok_r(NULL,":",&ptr2);
			tear->reason = atoi(header_value);
			//printf("reason:%d\n",tear->reason);
		}else if(strcmp(header,RTSP_HEADER_SESSION) == 0)
		{
			header_value = strtok_r(NULL,":",&ptr2);
			tear->session = atol(header_value);
			//printf("session:%llu\n",tear->session);
		}else if(strcmp(header,RTSP_HEADER_ONDEMANDSESSIONID) == 0)
		{
			header_value = strtok_r(NULL,":",&ptr2);
			strcpy(tear->ondemand_session_id,header_value);
			trim(tear->ondemand_session_id);
			//printf("ondemandsessionid:%s\n",tear->ondemand_session_id);
		}else if(strcmp(header,RTSP_HEADER_CLIENTSESSIONID) == 0)
		{
			header_value = strtok_r(NULL,":",&ptr2);
			strcpy(tear->client_session_id,header_value);
			trim(tear->client_session_id);
			//printf("client_session_id:%s\n",tear->client_session_id);
		}
		line = strtok_r(NULL,"\r\n",&ptr);	
	}
	return 0;	
}
//创建SM发给STB的TEARDOWN RESPONSE消息
int rtsp_s1_teardown_res_encode(S1_TEARDOWN_RES res,char *tear_res)
{
	char str[1024];
	sprintf(str,"RTSP/1.0 %d OK\r\n"
				"CSeq: %d\r\n"
				"Session: %llu\r\n"
				"OnDemandSessionId: %s\r\n"
				"ClientSessionId: %s\r\n\r\n"
			,res.err_code,res.cseq,res.session,res.ondemand_session_id,res.client_session_id);
	strcpy(tear_res,str);
	return 0;	
}
//创建SM发给STB的ANNOUNCE消息
int rtsp_s1_announce_msg_encode(S1_ANNOUNCE_MSG ann,char *ann_msg)
{
	char str[1024] = "";
	char notice_des[30] = "";
	rtsp_notice_description(ann.notice,notice_des);
	printf("%s\n",notice_des);
	
	sprintf(str,"ANNOUNCE rtsp://%s:%d  RTSP/1.0\r\n"
				"CSeq: %d\r\n"
				"Require: %s\r\n"
				"Session: %llu\r\n"
				"Notice: %d %s event-date=%s npt=%s\r\n"
				"OnDemandSessionId: %s\r\n\r\n",
			ann.sm_ip,ann.sm_port,ann.cseq,ann.require,ann.session,ann.notice,notice_des,ann.event_date,ann.npt,ann.ondemand_session_id);
	strcpy(ann_msg,str);
	return 0;	
}
//解析STB发给SM的Announce Response消息
int rtsp_s1_announce_res_parse(char *ann_res,S1_ANNOUNCE_RES *ann)
{
	char str[1024];
	char *line = NULL;
	char *header = NULL;
	char *header_value = NULL;
	char *ptr = NULL;
	char *ptr1 = NULL;
	char *ptr2 = NULL;
	
	strncpy(str,ann_res,strlen(ann_res)-4);
	line = strtok_r(str,"\r\n",&ptr);
	strtok_r(line," ",&ptr1);
	ann->err_code = atoi(strtok_r(NULL," ",&ptr1));//解析err_code
	printf("%d\n",ann->err_code);
	
	line = strtok_r(NULL,"\r\n",&ptr);
	while(line)
	{
		//printf("line:%s\n",line);
		header = strtok_r(line,":",&ptr2);
		if(strcmp(header,RTSP_HEADER_CSEQ) == 0)
		{
			header_value = strtok_r(NULL,":",&ptr2);
			ann->cseq = atoi(header_value);
			printf("cseq:%d\n",ann->cseq);	
		}else if(strcmp(header,RTSP_HEADER_SESSION) == 0)
		{
			header_value = strtok_r(NULL,":",&ptr2);
			ann->session = atol(header_value);
			printf("session:%llu\n",ann->session);	
		}else if(strcmp(header,RTSP_HEADER_ONDEMANDSESSIONID) == 0)
		{
			header_value = strtok_r(NULL,":",&ptr2);
			strcpy(ann->ondemand_session_id,header_value);
			trim(ann->ondemand_session_id);
			printf("ondemandsessionid:%s\n",ann->ondemand_session_id);	
		}
		
		
		line = strtok_r(NULL,"\r\n",&ptr);
	}	
    return 0;	
}
//解析STB发给SM的PING消息
int rtsp_s1_ping_msg_parse(char *ping_msg,S1_PING_MSG *ping)
{
	char str[1024] = "";
	char *method = NULL;
	char *url = NULL;
	char *rtsp_version = NULL;
	char *line = NULL;
	char *header = NULL;
	char *header_value = NULL;
	char *ptr = NULL;
	char *ptr1 = NULL;
	char *ptr2 = NULL;
	
	strncpy(str,ping_msg,strlen(ping_msg)-4);//复制teardown消息（不复制消息最后的"\r\n\r\n"）
	//解析消息第一行
	line = strtok_r(str,"\r\n",&ptr);
	method = strtok_r(line," ",&ptr1);
	if(strcmp(method,RTSP_METHOD_PING))
	{
		printf("Method is not matched!");
		return -1;
	}
	url = strtok_r(NULL," ",&ptr1);
	trim(url);
	parse_url(url,ping->sm_ip,&(ping->sm_port),NULL);//解析出消息中的sm_ip和sm_port
	rtsp_version = strtok_r(NULL," ",&ptr1);
	if(strcmp(rtsp_version,RTSP_VERSION))
	{
		printf("RTSP_VERSION is not matched!");
		return -1;	
	}
	printf("%s %d\n",ping->sm_ip,ping->sm_port);
	//按行解析剩余消息
	line = strtok_r(NULL,"\r\n",&ptr);	
	while(line != NULL)
	{	
		header = strtok_r(line,":",&ptr2);
		if(strcmp(header,RTSP_HEADER_CSEQ) == 0)
		{
			header_value = strtok_r(NULL,":",&ptr2);
			ping->cseq = atoi(header_value);
			printf("cseq:%d\n",ping->cseq);
		}else if(strcmp(header,RTSP_HEADER_REQUIRE) == 0)
		{
			header_value = strtok_r(NULL,":",&ptr2);
			strcpy(ping->require,header_value);
			trim(ping->require);
			printf("require:%s\n",ping->require);
		}else if(strcmp(header,RTSP_HEADER_SESSION) == 0)
		{
			header_value = strtok_r(NULL,":",&ptr2);
			ping->session = atol(header_value);
			printf("session:%llu\n",ping->session);
		}else if(strcmp(header,RTSP_HEADER_ONDEMANDSESSIONID) == 0)
		{
			header_value = strtok_r(NULL,":",&ptr2);
			strcpy(ping->ondemand_session_id,header_value);
			trim(ping->ondemand_session_id);
			printf("ondemandsessionid:%s\n",ping->ondemand_session_id);
		}
		line = strtok_r(NULL,"\r\n",&ptr);	
	}
	return 0;	
	
}
//创建SM发给STB的Ping Response消息
int rtsp_s1_ping_res_encode(S1_PING_RES ping,char *ping_res)
{
	char str[1024];
	sprintf(str,"RTSP/1.0 %d OK\r\n"
				"CSeq: %d\r\n"
				"Session: %llu\r\n"
				"OnDemandSessionId: %s\r\n\r\n",
			ping.err_code,ping.cseq,ping.session,ping.ondemand_session_id);
	strcpy(ping_res,str);
	return 0;	
}


