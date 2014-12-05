#include "sm_rtsp_s6_msg_process.h"
#include "sm_rtsp_public_function.h"

//积极模式下或悲观模式下创建SM发给ERM的SETUP信息
int rtsp_s6_setup_msg_encode(S6_SETUP_MSG msg,char *setup_msg)
{
    char str[1024] = "";
    char localip[40] = "";
    char transport[1024] = "";
    char qam_info[256] = "";
    int i = 0;
    
    //首先生成Transport头
    for (i=0;i<msg.qam_num;i++)
    {
        sprintf(qam_info,"MP2T/DVBC/QAM;unicast;bandwidth=%llu;qam_name=%s;client=%s;modulation=%s",
        		msg.qam_info[i].bandwidth,msg.qam_info[i].qam_name,msg.qam_info[i].client,msg.qam_info[i].modulation);            
        strcat(transport,qam_info);
        if (i != msg.qam_num-1)
        {
            strcat(transport,",");
        }
        memset(qam_info,0x00,sizeof(qam_info));
    }
    //生成SETUP消息
    sprintf(str,"SETUP rtsp://%s RTSP/1.0\r\n"
            "CSeq: %d\r\n"
            "Require: %s\r\n"
            "SessionGroup: %s\r\n"
            "EncryptionType: %s\r\n"
            "CAS_ID: %s\r\n"
            "EncryptControl: %s\r\n"
            "Transport:%s\r\n"
            "OnDemandSessionId: %s\r\n"
            "Policy: %s\r\n"
            "InbandMarker: %s\r\n\r\n",
            msg.ip,msg.cseq,msg.require,msg.session_group,msg.encryption_type,msg.cas_id,msg.encrypt_control,
            transport,msg.ondemandsessionid,msg.policy,msg.inband_marker);        
    strcpy(setup_msg,str);
    return 0;
}
//悲观模式下解析ERM作为RTSP代理向SM发送的SETUP消息
int rtsp_s6_pes_setup_msg_parse(char *setup_msg,S6_SETUP_MSG1 *msg1)
{
    char url[100] = "";
    char ip[40] = "";
    char localip[40]="";
    char *header = NULL;
    char *header_value = NULL;
    char *subheader = NULL;
    char *subheader_value= NULL;
    char *find = NULL;
    char *findend = NULL;
    char *substr = (char *)malloc(strlen(setup_msg) + 1);
    memset(substr,0x00,sizeof(substr));
    char *_substr = (char *)malloc(strlen(setup_msg) + 1);
    memset(_substr,0x00,sizeof(_substr));
    char *line = (char *)malloc(sizeof(char)*300);
    memset(line,0x00,sizeof(line));
    char *_line = (char *)malloc(sizeof(char)*300);
    memset(_line,0x00,sizeof(_line));
    char *subline;
    char *out_ptr = NULL;
    char *in_ptr = NULL;
    char *inner_ptr = NULL;

    find = strstr(setup_msg,"\r\n");
    strncpy(line,setup_msg,find-setup_msg);//获取消息第一行
    strcpy(substr,setup_msg+(find-setup_msg)+2);
    //解析消息第一行
    if (strstr(line,"SETUP"))
    {
        strcpy(_line,line+6);
        find = strstr(_line,"rtsp://");
        findend = strstr(_line,RTSP_VERSION);
        if (findend == NULL)
        {
            printf("RTSP_VERSION is not matched!\n");
            return -1;//版本号不相符，返回错误
        }
        strncpy(url,_line+(find-_line),(findend-find));//截取URL
        trim(url);
        parse_url(url,ip,NULL,NULL);
        getlocalip("eth0",localip);
        if (strcmp(ip,localip) != 0)   //查看消息中URL地址是否和自己匹配
        {
            printf("Host IP is not matched!\n");
            return -1;//消息中URL地址不和自己地址匹配，返回错误
        }
    }
    else
    {
        printf("Method is wrong!\n");
        return -1;//没有找到"SETUP"消息头,返回错误
    }
    while (1)
    {
        find = strstr(substr,"\r\n");
        strncpy(line,substr,find-substr);//获取下一个消息头信息
        strcpy(_substr,substr+(find-substr)+2);//保存未parse的消息

        //按行解析消息
        {
            header= NULL;
            header_value = NULL;
            header = trim(strtok(line,":"));
            header_value = trim(strtok(NULL,":"));
            if (strcmp(header,RTSP_HEADER_CSEQ) == 0)
            {
                msg1->cseq = atoi(header_value);
            }
            else if (strcmp(header,RTSP_HEADER_REQUIRE) == 0)
            {
                strcpy(msg1->require,header_value);
            }
            else if (strcmp(header,RTSP_HEADER_SESSION_GUOUP) == 0)
            {
                strcpy(msg1->session_group,header_value);
            }
            else if (strcmp(header,RTSP_HEADER_TRANSPORT) == 0)   //解析Transport头
            {
                int i = 0;
                subline = strtok_r(header_value,",",&out_ptr);
                while (subline)
                {
                    subheader = strtok_r(subline,";",&in_ptr);
                    while (subheader != NULL)
                    {
                        if (strtok_r(subheader,"=",&inner_ptr))
                        {
                            if (strcmp(subheader,"destination") == 0)
                            {
                                subheader_value = strtok_r(NULL,"=",&inner_ptr);
                                strcpy(msg1->qam_info1[i].destination,subheader_value);
                                printf("%s\n",msg1->qam_info1[i].destination);
                            }
                            else if (strcmp(subheader,"client_port") == 0)
                            {
                                subheader_value = strtok_r(NULL,"=",&inner_ptr);
                                msg1->qam_info1[i].client_port = atoi(subheader_value);
                                printf("%d\n",msg1->qam_info1[i].client_port);
                            }
                            else if (strcmp(subheader,"client") == 0)
                            {
                                subheader_value = strtok_r(NULL,"=",&inner_ptr);
                                strcpy(msg1->qam_info1[i].client,subheader_value);
                                printf("%s\n",msg1->qam_info1[i].client);
                            }

                        }
                        subheader = strtok_r(NULL,";",&in_ptr);
                    }
                    i++;
                    msg1->qam_num = i;
                    printf("qam_num:%d\n",msg1->qam_num);
                    subline = strtok_r(NULL,",",&out_ptr);

                }


            }
            else if (strcmp(header,RTSP_HEADER_ONDEMANDSESSIONID) == 0)
            {
                strcpy(msg1->ondemandsessionid,header_value);
            }
            else if (strcmp(header,RTSP_HEADER_POLICY) == 0)
            {
                strcpy(msg1->policy,header_value);
            }
        }

        if (strcmp(_substr,"\r\n") != 0)   //判断消息是否到结尾（因消息结尾有两个"\r\n"，按行解析消息时会留下一个"\r\n"）
        {
            memset(substr,0x00,strlen(substr));
            strcpy(substr,_substr);
        }
        else break;

        memset(line,0x00,sizeof(char)*300);
        memset(_substr,0x00,strlen(_substr));
    }

    free(line);
    free(_line);
    free(substr);
    return 0;
}
//悲观模式下SM向ERM发送的SETUP Response消息
int rtsp_s6_pes_setup_res_encode(S6_SETUP_RES1 res1,char *res1_msg)
{
    char str[1024] = "";

    if (res1.err_code == 200)
    {
        //生成SETUP消息
        sprintf(str,"RTSP/1.0 200 OK\r\n"
                "CSeq: %d\r\n"
                "Session: %llu\r\n"
                "Transport:MP2T/DVBC/UDP;unicast;destination=%s;client_port=%d;source=%s;server_port=%d;client=%s\r\n"
                "EmbeddedEncryptor: %s\r\n"
                "OnDemandSessionId: %s\r\n\r\n",
                res1.cseq,res1.session,res1.destination,res1.client_port,res1.source,res1.server_port,res1.client,res1.embedded_encryptor,res1.ondemandsessionid);
        strcpy(res1_msg,str);
    }
    else
    {
        rtsp_err_res_encode(res1.err_code,res1.cseq,res1_msg);
    }

    return 0;
}
//积极模式下或悲观模式下解析ERM向SM发送的Response消息
int rtsp_s6_setup_res_parse(char *msg,S6_SETUP_RES *res)
{
    char str[1024]="";
    char *rtsp_version= NULL;
    char *line = NULL;
    char *header = NULL;
    char *header_value = NULL;
    char *subheader = NULL;
    char *subheader_value = NULL;
    char *ptr = NULL;
    char *ptr1 =NULL;
    char *ptr2 = NULL;
    char *ptr3 = NULL;
    char *ptr4 = NULL;

    strncpy(str,msg,strlen(msg)-4);//去除消息最后的“\r\n\r\n”
    //strcpy(str,msg);
    //printf("%s\n",str);

    line = strtok_r(str,"\r\n",&ptr);
    //printf("line:%s\n",line);
    //解析第一行
    rtsp_version = strtok_r(line," ",&ptr1);//解析RTSP版本号
   // if (strcmp(rtsp_version,RTSP_VERSION) != 0)
    //{
      //  printf("RTSP_VERSION is not matched!\n");//如果版本号不符，返回-1
        //return -1;
   // }
    res->err_code = atoi(strtok_r(NULL," ",&ptr1));//解析消息错误码
    //if (res->err_code != RTSP_ResponseCode_OK)
    //{
      //  printf("RTSP_ERR_CODE is wrong!\n");//错误码不符，返回-1
        //return -1;
    //}
    //按行解析剩余消息
    line = strtok_r(NULL,"\r\n",&ptr);
    while (NULL != line)
    {
       // printf("line:%s\n",line);
        if (header = strtok_r(line,":",&ptr2))
        {
            if (strcmp(header,RTSP_HEADER_CSEQ) == 0)
            {
                header_value = strtok_r(NULL,":",&ptr2);
                res->cseq = atoi(header_value);
                //printf("cseq:%d\n",res->cseq);
            }
            else if (strcmp(header,RTSP_HEADER_SESSION) == 0)
            {
                header_value = strtok_r(NULL,":",&ptr2);
                res->session = atol(header_value);
                //printf("session:%llu\n",res->session);
            }
            else if (strcmp(header,RTSP_HEADER_TRANSPORT) == 0)
            {
                header_value = strtok_r(NULL,":",&ptr2);
                subheader = strtok_r(header_value,";",&ptr3);
                while (subheader != NULL)
                {
                    if (strtok_r(subheader,"=",&ptr4))
                    {
                        if (strcmp(subheader,"client") == 0)
                        {
                            subheader_value = strtok_r(NULL,"=",&ptr4);
                            strcpy(res->client,subheader_value);
                            //printf("%s\n",res->client);
                        }else if (strcmp(subheader,"destination") == 0)
                        {
                            subheader_value = strtok_r(NULL,"=",&ptr4);
                            strcpy(res->destination,subheader_value);
                            //printf("%s\n",res->destination);
                        }else if (strcmp(subheader,"client_port") == 0)
                        {
                            subheader_value = strtok_r(NULL,"=",&ptr4);
                            res->client_port = atoi(subheader_value);
                            //printf("%d\n",res->client_port);
                        }else if (strcmp(subheader,"qam_destination") == 0)
                        {
                            subheader_value = strtok_r(NULL,"=",&ptr4);
                            strcpy(res->qam_destination,subheader_value);
                            //printf("%s\n",res->qam_destination);
                        }else if (strcmp(subheader,"qam_name") == 0)
                        {
                            subheader_value = strtok_r(NULL,"=",&ptr4);
                            strcpy(res->qam_name,subheader_value);
                            //printf("%s\n",res->qam_name);
                        }else if (strcmp(subheader,"qam_group") == 0)
                        {
                            subheader_value = strtok_r(NULL,"=",&ptr4);
                            strcpy(res->qam_group,subheader_value);
                            //printf("%s\n",res->qam_group);
                        }else if (strcmp(subheader,"edge_input_group") == 0)
                        {
                            subheader_value = strtok_r(NULL,"=",&ptr4);
                            strcpy(res->edge_input_group,subheader_value);
                            //printf("%s\n",res->edge_input_group);
                        }                                                    
                    }
                    subheader = strtok_r(NULL,";",&ptr3);
                }

            }
            else if (strcmp(header,RTSP_HEADER_EMBEDDED_ENCRYPTOR) == 0)
            {
                header_value = strtok_r(NULL,":",&ptr2);
                strcpy(res->embedded_encryptor,header_value);
                trim(res->embedded_encryptor);
                //printf("embedded_encryptor:%s\n",res->embedded_encryptor);
            }
            else if (strcmp(header,RTSP_HEADER_ONDEMANDSESSIONID) == 0)
            {
                header_value = strtok_r(NULL,":",&ptr2);
                strcpy(res->ondemandsessionid,header_value);
                trim(res->ondemandsessionid);
                //printf("ondemandsessionid:%s\n",res->ondemandsessionid);
            }
        }
        line = strtok_r(NULL,"\r\n",&ptr);
    }

    return 0;
}
//创建SM向ERM发送的TEARDOWN消息
int rtsp_s6_teardown_msg_encode(S6_TEARDOWN_MSG1 msg1,char *tear_msg)
{
	char str[1024] = "";
	char reason[50] ="";
	rtsp_reason_description(msg1.reason,reason); 
	sprintf(str,"TEARDOWN rtsp://%s RTSP/1.0\r\n"
				"CSeq: %d\r\n"
				"Require: com.comcast.ngod.s6\r\n"
				"Reason: %d \"%s\"\r\n"
				"Session: %llu\r\n"
				"OnDemandSessionId: %s\r\n\r\n"
				,msg1.rtsp_ip,msg1.cseq,msg1.reason,reason,msg1.session,msg1.ondemandsessionid);
	strcpy(tear_msg,str);
	return 0;	
}
//解析ERM作为代理向SM发送的TEARDOWN消息
int rtsp_s6_teardown_msg_parse(char *tear_msg,S6_TEARDOWN_MSG2 *msg2)
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
	parse_url(url,msg2->rtsp_ip,NULL,NULL);//解析出消息中的rtsp_ip
	rtsp_version = strtok_r(NULL," ",&ptr1);
	if(strcmp(rtsp_version,RTSP_VERSION))
	{
		printf("RTSP_VERSION is not matched!");
		return -1;	
	}
	printf("%s\n",msg2->rtsp_ip);
	//按行解析剩余消息
	line = strtok_r(NULL,"\r\n",&ptr);	
	while(line != NULL)
	{	
		header = strtok_r(line,":",&ptr2);
		if(strcmp(header,RTSP_HEADER_CSEQ) == 0)
		{
			header_value = strtok_r(NULL,":",&ptr2);
			msg2->cseq = atoi(header_value);
			printf("cseq:%d\n",msg2->cseq);
		}else if(strcmp(header,RTSP_HEADER_REQUIRE) == 0)
		{
			header_value = strtok_r(NULL,":",&ptr2);
			strcpy(msg2->require,header_value);
			trim(msg2->require);
			printf("require:%s\n",msg2->require);
		}else if(strcmp(header,RTSP_HEADER_REASON) == 0)
		{
			header_value = strtok_r(NULL,":",&ptr2);
			msg2->reason = atoi(header_value);
			printf("reason:%d\n",msg2->reason);
		}else if(strcmp(header,RTSP_HEADER_SESSION) == 0)
		{
			header_value = strtok_r(NULL,":",&ptr2);
			msg2->session = atol(header_value);
			printf("session:%llu\n",msg2->session);
		}else if(strcmp(header,RTSP_HEADER_ONDEMANDSESSIONID) == 0)
		{
			header_value = strtok_r(NULL,":",&ptr2);
			strcpy(msg2->ondemandsessionid,header_value);
			trim(msg2->ondemandsessionid);
			printf("ondemandsessionid:%s\n",msg2->ondemandsessionid);
		}
		line = strtok_r(NULL,"\r\n",&ptr);	
	}
	return 0;	
}
//创建SM发给ERM的Teardown Response
int rtsp_s6_teardown_res_encode(S6_TEARDOWN_RES1 res1,char *tear_res)
{
	char str[1024];
	sprintf(str,"RTSP/1.0 %d OK\r\n"
				"CSeq: %d\r\n"
				"Session: %llu\r\n"
				"OnDemandSessionId: %s\r\n\r\n"
			,res1.err_code,res1.cseq,res1.session,res1.ondemandsessionid);
	strcpy(tear_res,str);
	return 0;
}
//解析ERM发给SM的Teardown Response
int rtsp_s6_teardown_res_parse(char *tear_res,S6_TEARDOWN_RES2 *res2)
{
	char str[1024];
	char *line = NULL;
	char *header = NULL;
	char *header_value = NULL;
	char *ptr = NULL;
	char *ptr1 = NULL;
	char *ptr2 = NULL;
	
	strncpy(str,tear_res,strlen(tear_res)-4);
	line = strtok_r(str,"\r\n",&ptr);
	strtok_r(line," ",&ptr1);
	res2->err_code = atoi(strtok_r(NULL," ",&ptr1));//解析err_code
	//printf("%d\n",res2->err_code);
	
	line = strtok_r(NULL,"\r\n",&ptr);
	while(line)
	{
		//printf("line:%s\n",line);
		header = strtok_r(line,":",&ptr2);
		if(strcmp(header,RTSP_HEADER_CSEQ) == 0)
		{
			header_value = strtok_r(NULL,":",&ptr2);
			res2->cseq = atoi(header_value);
			//printf("cseq:%d\n",res2->cseq);	
		}else if(strcmp(header,RTSP_HEADER_SESSION) == 0)
		{
			header_value = strtok_r(NULL,":",&ptr2);
			res2->session = atol(header_value);
			//printf("session:%llu\n",res2->session);	
		}else if(strcmp(header,RTSP_HEADER_ONDEMANDSESSIONID) == 0)
		{
			header_value = strtok_r(NULL,":",&ptr2);
			strcpy(res2->ondemandsessionid,header_value);
			trim(res2->ondemandsessionid);
			//printf("ondemandsessionid:%s\n",res2->ondemandsessionid);	
		}
		
		
		line = strtok_r(NULL,"\r\n",&ptr);
	}	
    return 0;
}
//解析ERM向SM发送的ANNOUNCE消息
int rtsp_s6_announce_msg_parse(char * announce,S6_ANNOUNCE_MSG *ann)
{
	char str[1024] = "";
	char *method = NULL;
	char *url = NULL;
	char *rtsp_version = NULL;
	char *line = NULL;
	char *header = NULL;
	char *header_value = NULL;
	char *subheader = NULL;
	char *subheader_value = NULL;
	char *ptr = NULL;
	char *ptr1 = NULL;
	char *ptr2 = NULL;
	char *ptr3 = NULL;
	char *ptr4 = NULL;
	
	strncpy(str,announce,strlen(announce)-4);//复制announce消息（不复制消息最后的"\r\n\r\n"）
	//解析消息第一行
	line = strtok_r(str,"\r\n",&ptr);
	method = strtok_r(line," ",&ptr1);
	if(strcmp(method,RTSP_METHOD_ANNOUNCE))
	{
		printf("Method is not matched!");
		return -1;
	}
	url = strtok_r(NULL," ",&ptr1);
	trim(url);
	parse_url(url,ann->rtsp_ip,NULL,NULL);//解析出消息中的rtsp_ip
	rtsp_version = strtok_r(NULL," ",&ptr1);
	if(strcmp(rtsp_version,RTSP_VERSION))
	{
		printf("RTSP_VERSION is not matched!");
		return -1;	
	}
	printf("%s\n",ann->rtsp_ip);
	
	//按行解析剩余消息
	line = strtok_r(NULL,"\r\n",&ptr);	
	while(line != NULL)
	{	
		header = strtok_r(line,":",&ptr2);
		if(strcmp(header,RTSP_HEADER_CSEQ) == 0)
		{
			header_value = strtok_r(NULL,":",&ptr2);
			ann->cseq = atoi(header_value);
			printf("cseq:%d\n",ann->cseq);
		}else if(strcmp(header,RTSP_HEADER_REQUIRE) == 0)
		{
			header_value = strtok_r(NULL,":",&ptr2);
			strcpy(ann->require,header_value);
			trim(ann->require);
			printf("require:%s\n",ann->require);
		}else if(strcmp(header,RTSP_HEADER_SESSION) == 0)
		{
			header_value = strtok_r(NULL,":",&ptr2);
			ann->session = atol(header_value);
			printf("session:%llu\n",ann->session);
		}else if(strcmp(header,RTSP_HEADER_NOTICE) == 0)
		{
			header_value = strtok_r(NULL,":",&ptr2);
			ann->notice = atoi(header_value);
			
			subheader =  strtok_r(header_value," ",&ptr3);
			while(subheader)
			{
			   // printf("subheader:%s\n",subheader);
			    if(strstr(subheader,"="))
			    {
			        strtok_r(subheader,"=",&ptr4);
			        if(strcmp(subheader,"event-date") == 0)
			        {
			            subheader_value = strtok_r(NULL,"=",&ptr4);
			            strcpy(ann->event_date,subheader_value);
			            printf("event_date:%s\n",ann->event_date);
			        }else if(strcmp(subheader,"npt") == 0)
			        {
			            subheader_value = strtok_r(NULL,"=",&ptr4);
			            if(subheader_value != NULL)
			            {
			                strcpy(ann->npt,subheader_value);
			                printf("npt:%s\n",ann->npt);
			            }
			        }
			        
			    }
			    subheader =  strtok_r(NULL," ",&ptr3);
			}

		}else if(strcmp(header,RTSP_HEADER_ONDEMANDSESSIONID) == 0)
		{
			header_value = strtok_r(NULL,":",&ptr2);
			strcpy(ann->ondemandsessionid,header_value);
			trim(ann->ondemandsessionid);
			printf("ondemandsessionid:%s\n",ann->ondemandsessionid);
		}
		line = strtok_r(NULL,"\r\n",&ptr);	
	}
	return 0;	
}
//创建SM向ERM发送的ANNOUNCE RESPONSE消息
int rtsp_s6_announce_res_encode(S6_ANNOUNCE_RES res,char *ann_res)
{
	char str[1024];
	sprintf(str,"RTSP/1.0 %d OK\r\n"
				"CSeq: %d\r\n"
				"Session: %llu\r\n"
				"OnDemandSessionId: %s\r\n\r\n"
			,res.err_code,res.cseq,res.session,res.ondemandsessionid);
	strcpy(ann_res,str);
	return 0;
}










