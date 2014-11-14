#include <stdio.h>
#include "sm_rtsp_public_function.h"
#include "sm_rtsp_s7_msg_process.h"
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
int main()
{
	char str[1024];
	char restr[1024];
	memset(str,0x00,sizeof(str));
	S7_SETUP_MSG msg;
	S7_SETUP_RES remsg;
	memset(&msg,0x00,sizeof(msg));

	strcpy(msg.asm_ip,"127.0.0.1");
	msg.asm_port=554;
	msg.cseq=110;
	strcpy(msg.require,"com.comcast.ngod.s7");
	strcpy(msg.session_group,"SM1");
	strcpy(msg.ondemandsessionid,"0000111100");
	strcpy(msg.policy,"rtsp");
	strcpy(msg.app_id,"002345");
	msg.app_type=0;
	strcpy(msg.ss.client,"192.168.10.97");
	strcpy(msg.ss.destination,"192.168.10.34");
	msg.ss.client_port=35;

	rtsp_s7_setup_msg_encode(msg,str);

	int sockfd;
	struct sockaddr_in servaddr;

	sockfd=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	memset(&servaddr,0x00,sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	servaddr.sin_port=htons(9998);
	inet_pton(AF_INET,"127.0.0.1",&servaddr.sin_addr);
	connect(sockfd,(struct sockaddr *)&servaddr, sizeof(servaddr));
	write(sockfd,str,strlen(str));
	memset(restr,0x00,strlen(restr));
	memset(&remsg,0x00,sizeof(remsg));
	int n=read(sockfd, restr, 1024);
    if(n > 0) 
    {
        printf("%s\n", restr);
    }
	rtsp_s7_setup_res_parse(restr,&remsg);
	return 0;
}
