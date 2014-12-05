#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <signal.h>
#include <arpa/inet.h>
#include <errno.h>
#include <iostream>
using namespace std;
int ConnectSock(int *pSock, int nPort, char * pAddr)
{


	struct sockaddr_in addrin;	
	long lAddr;	
	int nSock;	
	int nRetVal = -1;

	assert(pSock != NULL && nPort > 0 && pAddr != NULL);	
	assert((nSock = socket(AF_INET, SOCK_STREAM, 0)) > 0);

	memset(&addrin, 0, sizeof(addrin));	
	addrin.sin_family = AF_INET;
	addrin.sin_addr.s_addr = inet_addr(pAddr);
	addrin.sin_port = htons(nPort);

	if(nRetVal = connect(nSock, (struct sockaddr *)&addrin, sizeof(addrin)) == 0)
	{
		*pSock = nSock;		
		return 0;
	}
	else
	{
		close(nSock);
		return -1;
	}	
}

int rtsp_write(int fd, void *buffer, int nbytes)
{
	int n;
	n = write((int)fd, buffer, nbytes);
	return n;
}
int rtsp_read(int fd, void *buffer, int nbytes)
{
	int n;
	n=recv(fd,buffer,nbytes,0);
	return n;
}

int main()
{   
  
    int pSock;
    int nPort = 8888;
    char * pAddr = "127.0.0.1";
    char response[2048];
    int nread = -1;
    string str = "SETUP rtsp://192.26.2.2 RTSP/1.0\r\n"
"CSeq: 314\r\n"
"Require: ermi\r\n"
"Transport: DOCSIS/QAM;unicast;bit_rate=38000000;qam_id=456;depi_mode=docsis_mpt,DOCSIS/QAM;unicast;bit_rate=380;qam_id=123;depi_mode=docsis_mpt\r\n\r\n";
    char* a = new char[str.size()+1];
    strcpy(a,str.c_str());
    cout<< a <<endl;

    ConnectSock(&pSock,nPort,pAddr); 
    rtsp_write(pSock,a, strlen(a)+1);
    nread=rtsp_read(pSock, response, 2048);

    for(int i=0;i<nread;i++)
    {
    	cout <<response[i];
    }
    
    return 0;
}
