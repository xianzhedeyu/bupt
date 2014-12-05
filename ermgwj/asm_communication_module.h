
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

int write_fd(int fd, void *ptr, int nbytes, int sendfd);
int read_fd(int fd, void *ptr, int nbytes, int *recvfd);

int CreateSock(int* pSock, int nPort);

int AcceptSock(int * pSock, int nSock);

int ConnectSock(int *pSock, int nPort, char * pAddr);

void rtsp_close(int s);

int rtsp_write(int fd, void *buffer, int nbytes);

int rtsp_read(int fd, void *buffer, int nbytes);



