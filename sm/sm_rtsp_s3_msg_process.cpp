#include "sm_rtsp_s3_msg_process.h"
#include "sm_rtsp_public_function.h"

//创建SM发给ODRM的SETUP信息
int rtsp_s3_setup_msg_encode(S3_SETUP_MSG msg,char *setup_msg)
{
    char sdp[1024] = "";//存储sdp消息
    char sg_buff[100] = "";//存储sop_group消息头
    char transport[512] = "";//存储transport消息头
    char qam_info[100] = "";
    char str[1024] = "";
    int i = 0;

    //生成sop_group消息头
    while (strlen(msg.sop_group[i]) != 0) {
        strcat(sg_buff,"SopGroup: ");
        strcat(sg_buff,msg.sop_group[i]);
        strcat(sg_buff,"\r\n");
        i++;
    }

    //生成Transport消息头
    for (i=0;i<msg.qam_num;i++) {
        sprintf(qam_info,"MP2T/DVBC/UDP;unicast;client=%s;bandwidth=%llu;destination=%s;client_port=%d",
                msg.qam[i].client,msg.qam[i].bandwidth,msg.qam[i].destination,msg.qam[i].client_port);
        strcat(transport,qam_info);
        if (i != msg.qam_num-1) {
            strcat(transport,",");
        }
        memset(qam_info,0x00,sizeof(qam_info));
    }

    //生成sdp消息
    sprintf(sdp,"v=%d\r\n"
            "o=%s %s %s %s %s %s\r\n"
            "s=%s\r\n"
            "t=%d %d\r\n"
            "a=X-playlist-item: %s %s %d-%d\r\n"
            "c=%s\r\n"
            "m=%s\r\n",
            msg.sdp_version,msg.email_add,msg.ondemand_session_id,msg.ntp,msg.add_type,msg.ip_version,msg.sm_ip,\
            msg.s,msg.time[0],msg.time[1],msg.provider_id,msg.asset_id,msg.start_npt,msg.stop_npt,msg.c,msg.m);

    sprintf(str,"SETUP rtsp://%s:%d RTSP/1.0\r\n"
            "CSeq: %d\r\n"
            "Require: %s\r\n"
            "OnDemandSessionId: %s\r\n"
            "%s"
            "Transport: %s\r\n"
            "SessionGroup: %s\r\n"
            "StartPoint: %d %s\r\n"
            "Policy: %s\r\n"
            "InbandMarker:%s\r\n"
            "Content-type: application/sdp\r\n"
            "Content-length: %d\r\n\r\n"
            "%s",
            msg.odrm_ip,msg.odrm_port,msg.cseq,msg.require,msg.ondemand_session_id,sg_buff,transport,msg.session_group,\
            msg.start_point_slot,msg.start_point_npt,msg.policy,msg.inband_marker,strlen(sdp),sdp);
    strcpy(setup_msg,str);

    return 0;
}

//解析ODRM发给SM的SETUP RESPONSE消息
int rtsp_s3_setup_res_parse(char *setup_res,S3_SETUP_RES *res)
{
    char buff[2048] = "";
    char str[1024] = "";
    char sdp[1024] = "";
    char *rtsp_version = NULL;
    char *line = NULL;
    char *header = NULL;
    char *header_value = NULL;
    char *subheader = NULL;
    char *subheader_value = NULL;
    char *find = NULL;
    char *ptr = NULL;
    char *ptr1 = NULL;
    char *ptr2 = NULL;
    char *ptr3 = NULL;
    char *ptr4 = NULL;


    strncpy(buff,setup_res,2048);
    find =  strstr(buff,"\r\n\r\n");
    strncpy(str,buff,find - buff);//将消息中sdp以外的消息存入str
    strcpy(sdp,find+4);//将消息sdp部分存入sdp
    //printf("str:\n%s\nsdp:\n%s\n",str,sdp);



    line = strtok_r(str,"\r\n",&ptr);
    //printf("line:%s\n",line);
    //解析第一行
    rtsp_version = strtok_r(line," ",&ptr1);//解析RTSP版本号
    if (strcmp(rtsp_version,RTSP_VERSION) != 0) {
        //printf("RTSP_VERSION is not matched!\n");//如果版本号不符，返回-1
        return -1;
    }
    res->err_code = atoi(strtok_r(NULL," ",&ptr1));//解析消息错误码
    if (res->err_code != RTSP_ResponseCode_OK) {
        //printf("RTSP_ERR_CODE is wrong!\n");//错误码不符，返回-1
        return -1;
    }
    //按行解析剩余消息
    line = strtok_r(NULL,"\r\n",&ptr);
    while (NULL != line) {
        // printf("line:%s\n",line);
        if (header = strtok_r(line,":",&ptr2)) {
            if (strcmp(header,RTSP_HEADER_CSEQ) == 0) {
                header_value = strtok_r(NULL,":",&ptr2);
                res->cseq = atoi(header_value);
                //printf("cseq:%d\n",res->cseq);
            } else if (strcmp(header,RTSP_HEADER_SESSION) == 0) {
                header_value = strtok_r(NULL,":",&ptr2);
                res->session = atol(header_value);
                //printf("session:%llu\n",res->session);
            } else if (strcmp(header,RTSP_HEADER_TRANSPORT) == 0) {
                header_value = strtok_r(NULL,":",&ptr2);
                subheader = strtok_r(header_value,";",&ptr3);
                while (subheader != NULL) {
                    if (strtok_r(subheader,"=",&ptr4)) {
                        if (strcmp(subheader,"client") == 0) {
                            subheader_value = strtok_r(NULL,"=",&ptr4);
                            strcpy(res->client,subheader_value);
                            //printf("%s\n",res->client);
                        } else if (strcmp(subheader,"destination") == 0) {
                            subheader_value = strtok_r(NULL,"=",&ptr4);
                            strcpy(res->destination,subheader_value);
                            //printf("%s\n",res->destination);
                        } else if (strcmp(subheader,"client_port") == 0) {
                            subheader_value = strtok_r(NULL,"=",&ptr4);
                            res->client_port = atoi(subheader_value);
                            //printf("%d\n",res->client_port);
                        } else if (strcmp(subheader,"source") == 0) {
                            subheader_value = strtok_r(NULL,"=",&ptr4);
                            strcpy(res->source,subheader_value);
                            //printf("%s\n",res->source);
                        } else if (strcmp(subheader,"server_port") == 0) {
                            subheader_value = strtok_r(NULL,"=",&ptr4);
                            res->server_port = atoi(subheader_value);
                            //printf("%d\n",res->server_port);
                        } else if (strcmp(subheader,"bandwidth") == 0) {
                            subheader_value = strtok_r(NULL,"=",&ptr4);
                            res->bandwidth = atol(subheader_value);
                            //printf("%d\n",res->bandwidth);
                        }
                    }
                    subheader = strtok_r(NULL,";",&ptr3);
                }
            } else if (strcmp(header,RTSP_HEADER_CONTENT_TYPE) == 0) {
                header_value = strtok_r(NULL,":",&ptr2);
                strcpy(res->content_type,header_value);
                trim(res->content_type);
                //printf("content_type:%s\n",res->content_type);
            } else if (strcmp(header,RTSP_HEADER_CONTENT_LENGTH) == 0) {
                header_value = strtok_r(NULL,":",&ptr2);
                res->content_length = atoi(header_value);
                //printf("content_length:%d\n",res->content_length);
            }
        }
        line = strtok_r(NULL,"\r\n",&ptr);
    }

//解析SDP消息
    line = strtok_r(sdp,"\r\n",&ptr);
    while (NULL != line) {
        if (header = strtok_r(line,"=",&ptr1)) {
			if(strcmp(header,"v") == 0){
				header_value = strtok_r(NULL,"=",&ptr1);
                res->sdp_version = atoi(header_value);
                //printf("sdp_version:%d\n",res->sdp_version);
			}else if(strcmp(header,"o") == 0){
				header_value = strtok_r(NULL,"=",&ptr1);
               	strcpy(res->email_add,strtok_r(header_value," ",&ptr2));
               	res->ss_session = atol(strtok_r(NULL," ",&ptr2));
               	strcpy(res->ntp,strtok_r(NULL," ",&ptr2));          
               	strcpy(res->add_type,strtok_r(NULL," ",&ptr2));               	
               	strcpy(res->ip_version,strtok_r(NULL," ",&ptr2));
               	strcpy(res->ss_ip,strtok_r(NULL," ",&ptr2));
               	                                  	
               	//printf("email_add:%s\n",res->email_add);	
               	//printf("ss_session:%llu\n",res->ss_session);
               	//printf("ntp:%s\n",res->ntp);
               	//printf("add_type:%s\n",res->add_type);
               	//printf("ip_version:%s\n",res->ip_version); 
               	//printf("ss_ip:%s\n",res->ss_ip);   	
			}else if(strcmp(header,"s") == 0){
				header_value = strtok_r(NULL,"=",&ptr1);
                strcpy(res->s,header_value);
                //printf("s:%s\n",res->s);
			}else if(strcmp(header,"t") == 0){
				header_value = strtok_r(NULL,"=",&ptr1);            
               	res->time[0] = atoi(strtok_r(header_value," ",&ptr2));
               	res->time[1] = atoi(strtok_r(NULL," ",&ptr2));              
               	//printf("t:%d %d\n",res->time[0],res->time[1]);   	
			}else if(strcmp(header,"a") == 0){
				header_value = strtok_r(NULL,"=",&ptr1);
				strtok_r(header_value,":/",&ptr2);
               	strcpy(res->protocol,strtok_r(NULL,":/",&ptr2));
               	strcpy(res->host,strtok_r(NULL,":/",&ptr2));                        	
               	res->port = atoi(strtok_r(NULL,":/",&ptr2));
               	res->stream_handle = atol(strtok_r(NULL,":/",&ptr2));
               	              	                                  	
               	//printf("protocol:%s\n",res->protocol);	
               	//printf("host:%s\n",res->host);
               	//printf("port:%d\n",res->port);
               	//printf("stream_handle:%llu\n",res->stream_handle);   	
			}else if(strcmp(header,"c") == 0){
				header_value = strtok_r(NULL,"=",&ptr1);
                strcpy(res->c,header_value);
                //printf("c:%s\n",res->c);
			}else if(strcmp(header,"m") == 0){
				header_value = strtok_r(NULL,"=",&ptr1);
                strcpy(res->m,header_value);
                //printf("m:%s\n",res->m);
			}
        }
		line = strtok_r(NULL,"\r\n",&ptr);
    }
    
    return 0;
}

//创建SM发给ODRM的TEARDOWN消息
int rtsp_s3_teardown_msg_encode(S3_TEARDOWN_MSG tear,char *tear_msg)
{
	char str[1024] = "";
	char reason[50] ="";
	rtsp_reason_description(tear.reason,reason); 
	sprintf(str,"TEARDOWN rtsp://%s:%d RTSP/1.0\r\n"
				"CSeq: %d\r\n"
				"Require: com.comcast.ngod.s3\r\n"
				"Reason: %d \"%s\"\r\n"
				"Session: %llu\r\n"
				"OnDemandSessionId: %s\r\n\r\n",
				tear.odrm_ip,tear.odrm_port,tear.cseq,tear.reason,reason,tear.session,tear.ondemand_session_id);
	strcpy(tear_msg,str);
	return 0;	
}

//解析ODRM发给SM的TEARDDOWN RESPONSE消息
int rtsp_s3_teardown_res_parse(char *tear_res,S3_TEARDOWN_RES *res)
{
	char str[1024];
	char *line = NULL;
	char *header = NULL;
	char *header_value = NULL;
	char *ptr = NULL;
	char *ptr1 = NULL;
	char *ptr2 = NULL;
	char *ptr3 = NULL;
	
	strncpy(str,tear_res,strlen(tear_res)-4);
	line = strtok_r(str,"\r\n",&ptr);
	strtok_r(line," ",&ptr1);
	res->err_code = atoi(strtok_r(NULL," ",&ptr1));//解析err_code
	//printf("%d\n",res->err_code);
	
	line = strtok_r(NULL,"\r\n",&ptr);
	while(line)
	{
		//printf("line:%s\n",line);
		header = strtok_r(line,":",&ptr2);
		if(strcmp(header,RTSP_HEADER_CSEQ) == 0)
		{
			header_value = strtok_r(NULL,":",&ptr2);
			res->cseq = atoi(header_value);
			//printf("cseq:%d\n",res->cseq);	
		}else if(strcmp(header,RTSP_HEADER_SESSION) == 0)
		{
			header_value = strtok_r(NULL,":",&ptr2);
			res->session = atol(header_value);
			//printf("session:%llu\n",res->session);	
		}else if(strcmp(header,RTSP_HEADER_ONDEMANDSESSIONID) == 0)
		{
			header_value = strtok_r(NULL,":",&ptr2);
			strcpy(res->ondemand_session_id,header_value);
			trim(res->ondemand_session_id);
			//printf("ondemandsessionid:%s\n",res->ondemand_session_id);	
		}else if(strcmp(header,RTSP_HEADER_STOP_POINT) == 0)
		{
			header_value = strtok_r(NULL,":",&ptr2);
			
			res->stop_point_slot = atoi(strtok_r(header_value," ",&ptr3));
			strcpy(res->stop_point_npt,strtok_r(NULL," ",&ptr3));
			trim(res->stop_point_npt);
			//printf("stop_point:%d %s\n",res->stop_point_slot,res->stop_point_npt);	
		}
		
		
		line = strtok_r(NULL,"\r\n",&ptr);
	}	
    return 0;
	
}

//解析ERM向SM发送的ANNOUNCE消息
int rtsp_s3_announce_msg_parse(char * announce,S3_ANNOUNCE_MSG *ann)
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
	parse_url(url,ann->sm_ip,&ann->sm_port,NULL);//解析出消息中的rtsp_ip
	rtsp_version = strtok_r(NULL," ",&ptr1);
	if(strcmp(rtsp_version,RTSP_VERSION))
	{
		printf("RTSP_VERSION is not matched!");
		return -1;	
	}
	printf("%s %d\n",ann->sm_ip,ann->sm_port);
	
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
			strcpy(ann->ondemand_session_id,header_value);
			trim(ann->ondemand_session_id);
			printf("ondemandsessionid:%s\n",ann->ondemand_session_id);
		}
		line = strtok_r(NULL,"\r\n",&ptr);	
	}
	return 0;	
}

//创建SM向ODRM发送的ANNOUNCE RESPONSE消息
int rtsp_s3_announce_res_encode(S3_ANNOUNCE_RES res,char *ann_res)
{
	char str[1024];
	sprintf(str,"RTSP/1.0 %d OK\r\n"
				"CSeq: %d\r\n"
				"Session: %llu\r\n"
				"OnDemandSessionId: %s\r\n\r\n"
			,res.err_code,res.cseq,res.session,res.ondemand_session_id);
	strcpy(ann_res,str);
	return 0;
}

//创建SM向ODRM发送的GET PARAMETER消息
int rtsp_s3_get_parameter_msg_encode(S3_GET_PARAMETER_MSG msg,char *get_parameter)
{
	char str[1024] = "";
	sprintf(str,"GET_PARAMETER rtsp://%s:%d RTSP/1.0\r\n"
			"CSeq: %d\r\n"
			"Require: com.comcast.ngod.s3\r\n"
			"Content-Type: %s\r\n"
			"Session: %llu\r\n"
			"Content-Length: %d\r\n\r\n"
			"%s\r\n",
			msg.odrm_ip,msg.odrm_port,msg.cseq,msg.content_type,msg.session,strlen(msg.parameter),msg.parameter);
	
	strncpy(get_parameter,str,1024);
	
	return 0;
}

//解析ODRM发给SM的GET_PARAMETER RESPONSE消息
int rtsp_s3_get_parameter_res_parse(char *get_parameter_res,S3_GET_PARAMETER_RES *res)
{
	char buff[1024];
	char str[1024];
	char parameter_val[300];
	char *line = NULL;
	char *header = NULL;
	char *header_value = NULL;
	char *find = NULL;
	char *ptr = NULL;
	char *ptr1 = NULL;
	char *ptr2 = NULL;
	char *ptr3 = NULL;
	
	strncpy(buff,get_parameter_res,1024);
    find =  strstr(buff,"\r\n\r\n");
    strncpy(str,buff,find - buff);
    strcpy(parameter_val,find+4);
    strncpy(res->parameter_val,parameter_val,strlen(parameter_val)-2);//去除parameter_value最后的"\r\n"
    printf("%s\n",res->parameter_val);
	
	line = strtok_r(str,"\r\n",&ptr);
	strtok_r(line," ",&ptr1);
	res->err_code = atoi(strtok_r(NULL," ",&ptr1));//解析err_code
	printf("%d\n",res->err_code);
	
	line = strtok_r(NULL,"\r\n",&ptr);
	while(line)
	{
		//printf("line:%s\n",line);
		header = strtok_r(line,":",&ptr2);
		if(strcmp(header,RTSP_HEADER_CSEQ) == 0)
		{
			header_value = strtok_r(NULL,":",&ptr2);
			res->cseq = atoi(header_value);
			printf("cseq:%d\n",res->cseq);	
		}else if(strcmp(header,RTSP_HEADER_SESSION) == 0)
		{
			header_value = strtok_r(NULL,":",&ptr2);
			res->session = atol(header_value);
			printf("session:%llu\n",res->session);	
		}else if(strcmp(header,RTSP_HEADER_CONTENT_TYPE) == 0)
		{
			header_value = strtok_r(NULL,":",&ptr2);
			strcpy(res->content_type,header_value);
			trim(res->content_type);
			printf("content_type:%s\n",res->content_type);	
		}else if(strcmp(header,RTSP_HEADER_CONTENT_LENGTH) == 0)
		{
			header_value = strtok_r(NULL,":",&ptr2);
			res->content_length = atoi(header_value);
			printf("content_length:%d\n",res->content_length);	
		}
				
		line = strtok_r(NULL,"\r\n",&ptr);
	}	
    return 0;
	
}

//创建SM向ODRM发送的SET PARAMETER消息
int rtsp_s3_set_parameter_msg_encode(S3_SET_PARAMETER_MSG msg,char *set_parameter)
{
	char str[1024] = "";
	sprintf(str,"SET_PARAMETER rtsp://%s:%d RTSP/1.0\r\n"
			"CSeq: %d\r\n"
			"Require: com.comcast.ngod.s3\r\n"
			"Content-Type: %s\r\n"
			"Content-Length: %d\r\n\r\n"
			"%s\r\n",
			msg.odrm_ip,msg.odrm_port,msg.cseq,msg.content_type,strlen(msg.content),msg.content);
	
	strncpy(set_parameter,str,1024);
	
	return 0;
}

//解析ODRM发给SM的SET_PARAMETER RESPONSE消息
int rtsp_s3_set_parameter_res_parse(char *set_parameter_res,S3_SET_PARAMETER_RES *res)
{
	char str[1024];
	char *line = NULL;
	char *header = NULL;
	char *header_value = NULL;
	char *ptr = NULL;
	char *ptr1 = NULL;
	char *ptr2 = NULL;
	
	strncpy(str,set_parameter_res,1024);
	line = strtok_r(str,"\r\n",&ptr);
	strtok_r(line," ",&ptr1);
	res->err_code = atoi(strtok_r(NULL," ",&ptr1));//解析err_code
	printf("%d\n",res->err_code);
	
	line = strtok_r(NULL,"\r\n",&ptr);
	while(line)
	{
		//printf("line:%s\n",line);
		header = strtok_r(line,":",&ptr2);
		if(strcmp(header,RTSP_HEADER_CSEQ) == 0)
		{
			header_value = strtok_r(NULL,":",&ptr2);
			res->cseq = atoi(header_value);
			printf("cseq:%d\n",res->cseq);	
		}
				
		line = strtok_r(NULL,"\r\n",&ptr);
	}	
    return 0;
	
}


