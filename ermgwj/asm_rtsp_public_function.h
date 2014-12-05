#ifndef SM_RTSP_PUBLIC_H
#define SM_RTSP_PUBLIC_H

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <uuid/uuid.h>
#include <time.h>
#include "public_def.h"



//产生ondemandsessionid
int ondemandsessionid_generate(char *id);
//生成NTP TIME
unsigned long NTP_time(time_t t);
//解析URL地址
int parse_url(char *url, char *ip,int *port,char *dir);
//获取本地主机IP地址
int getlocalip(char* eth,char* localip);
//删除字符串收尾空格
char *trim(char *str);
//获取消息类型
int rtsp_get_msg_type(char *msg);
//创建SM发给ERM的错误response消息
int rtsp_err_res_encode(int err_code,int cseq,char *err_msg);
//查询Teardown消息中Reason头中编码号对应的描述
int rtsp_reason_description(int reason_code,char *description);
//查询Announce消息中Notice头中编码号对应的描述
int rtsp_notice_description(int notice_code,char *description);


#endif


