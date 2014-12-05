#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include "asm_rtsp_s7_msg_process.h"

int main()
{
	int lisfd,connfd;
	socklen_t clilen;
	struct sockaddr_in cliaddr,servaddr;
	char str[1024];
	int m=-2;

	lisfd=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

	memset(&servaddr,0x00,sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	servaddr.sin_port=htons(9998);

	m=bind(lisfd,(struct sockaddr *)&servaddr,sizeof(servaddr));
	m=listen(lisfd,10);
	clilen=sizeof(cliaddr);
	connfd=accept(lisfd,(struct sockaddr*)&cliaddr,&clilen);
	int n=read(connfd,str,1024);
	if(n>0)
		printf("%s",str);
    S7_SETUP_RES resmsg;
    resmsg.err_code = 200;
    resmsg.cseq = 1;
    resmsg.cseq = 123456;
    strcpy(resmsg.ondemandsessionid, "asdfsdfsfdfds");
    strcpy(resmsg.ss.client, "127.0.0.1");
    strcpy(resmsg.ss.destination, "127.0.0.1");
    resmsg.ss.client_port = 2000;
    strcpy(resmsg.as.ip, "127.0.0.1");
    resmsg.as.downPort = 2000;

    char res[1024];
    memset(res, 0x00, strlen(res));
    rtsp_s7_setup_res_encode(resmsg, res);
    write(connfd, res, strlen(res));

	return 0;
}
