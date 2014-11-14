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
int CreateSock(int* pSock, int nPort) {
	struct sockaddr_in addrin;
	struct sockaddr *paddr = (struct sockaddr *) &addrin;
	const int on = 1;

	struct linger lingerStruct;
	lingerStruct.l_onoff = 1;
	lingerStruct.l_linger = 0;

	memset(&addrin, 0, sizeof(addrin));

	addrin.sin_family = AF_INET;
	addrin.sin_addr.s_addr = htonl(INADDR_ANY);
	addrin.sin_port = htons(nPort);/*nPort == 6069*/

	if ((*pSock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		return -1;
	}
	setsockopt(*pSock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	setsockopt(*pSock, SOL_SOCKET, SO_LINGER, (char *) &lingerStruct, sizeof(lingerStruct));
	if (bind(*pSock, paddr, sizeof(addrin)) >= 0) {
		listen(*pSock, 50) >= 0;
		return 0;
	} else
		close(*pSock);
	return -1;

}

int AcceptSock(int * pSock, int nSock) {
	struct sockaddr_in addrin;
	socklen_t lSize;
	char addr[16] = "";
	lSize = sizeof(addrin);
	memset(&addrin, 0, sizeof(addrin));

	if ((*pSock = accept(nSock, (struct sockaddr *) &addrin, &lSize)) > 0) {
		inet_ntop(AF_INET, &(addrin.sin_addr), addr, sizeof(addr));
		//fprintf(stderr,"connect form %s\n", addr);
		//log(LVLSYS,SYS_INFO,"Cpe form %s connecting...",addr);
	} else {
		return -1;
	}
	return 0;
}
int ConnectSock(int *pSock, int nPort, char * pAddr) {

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

	if (nRetVal = connect(nSock, (struct sockaddr *) &addrin, sizeof(addrin)) == 0) {
		*pSock = nSock;
		return 0;
	} else {
		close(nSock);
		return -1;
	}
}

int rtsp_write(int fd, void *buffer, int nbytes) {
	int n;
	n = write((int) fd, buffer, nbytes);
	return n;
}
int rtsp_read(int fd, void *buffer, int nbytes) {
	int n;
	n = recv(fd, buffer, nbytes, 0);
	return n;
}

int main() {
	int sd;
	int pSock;
	int nPort = 8765;
	char * pAddr = "127.0.0.1";
	char response[2048];
	int nread = -1;
	string str = "RTSP/1.0 200 OK\r\n"
"CSeq: 102\r\n"
"Session: 47112344\r\n\r\n";
	char* a = new char[str.size() + 1];
	strcpy(a, str.c_str());
	cout << a << endl;

	CreateSock(&pSock, nPort);
	while (1) {
		AcceptSock(&sd, pSock);
		nread = rtsp_read(sd, response, 2048);

		for (int i = 0; i < nread; i++) {
			cout << response[i];
		}
		rtsp_write(sd,a, strlen(a)+1);
	}
	//rtsp_write(pSock, a, strlen(a) + 1);


	return 0;
}
