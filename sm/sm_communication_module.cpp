#include"sm_communication_module.h"
#include"sm_log.h"

int CreateSock(int* pSock, int nPort)
{
	struct sockaddr_in addrin;	
	struct sockaddr *paddr = (struct sockaddr *)&addrin;
	const int on = 1;
//没有延迟，立即终止
	struct linger lingerStruct;
	  lingerStruct.l_onoff   =   1;
	  lingerStruct.l_linger   =   0;


	memset(&addrin, 0, sizeof(addrin));

	addrin.sin_family = AF_INET;
	addrin.sin_addr.s_addr = htonl(INADDR_ANY);
	addrin.sin_port = htons(nPort);

	if((*pSock = socket(AF_INET, SOCK_STREAM, 0))< 0)
	{
		return -1;
	}
	setsockopt(*pSock,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
	setsockopt(*pSock,SOL_SOCKET,SO_LINGER,(char *)&lingerStruct, sizeof(lingerStruct));
	if(bind(*pSock,paddr, sizeof(addrin))>=0)
	{
		listen(*pSock,50)>=0;
		return 0;
	}
	else
		close(*pSock);
	return -1;

}

int AcceptSock(int * pSock, int nSock)
{
	struct sockaddr_in addrin;
	socklen_t lSize;
	char addr[16]="";
	lSize = sizeof(addrin);
	memset(&addrin, 0, sizeof(addrin));

	if ((*pSock = accept(nSock, (struct sockaddr *)&addrin, &lSize)) > 0){
		inet_ntop(AF_INET,&(addrin.sin_addr),addr,sizeof(addr));
		//fprintf(stderr,"connect from %s\n", addr);
		//sm_log(LVLSYS,SYS_INFO,"Cpe form %s connecting...",addr);
	}
	else{
		return -1;
	}
	return 0;
}

int ConnectSock(int *pSock, int nPort, char * pAddr)
{
	struct sockaddr_in addrin;	
	long lAddr;	
	int nSock;	
	int nRetVal = -1;

	if(nPort <= 0 || pAddr == NULL)
		return -2;
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

void rtsp_close(int s)
{

	close(s);

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

/*
UNIX 域套接字接口
负责传递描述符
*/

int write_fd(int fd, void *ptr, int nbytes, int sendfd)
{
        struct msghdr msg;
        struct iovec iov[1];
        union{
                struct cmsghdr cm;
                char control[CMSG_SPACE(sizeof(int))];
        }control_un;
        struct cmsghdr *cmptr;
        msg.msg_control = control_un.control;
        msg.msg_controllen = sizeof(control_un.control);
        cmptr = CMSG_FIRSTHDR(&msg);
        cmptr->cmsg_len = CMSG_LEN(sizeof(int));
        cmptr->cmsg_level = SOL_SOCKET;
        cmptr->cmsg_type = SCM_RIGHTS;
        *((int*)CMSG_DATA(cmptr)) = sendfd;
        msg.msg_name = NULL;
        msg.msg_namelen = 0;
        iov[0].iov_base = ptr;
        iov[0].iov_len = nbytes;
        msg.msg_iov = iov;
        msg.msg_iovlen = 1;
        return sendmsg(fd, &msg, 0);
}
int read_fd(int fd, void *ptr, int nbytes, int *recvfd)
{
    struct msghdr msg;
    struct iovec iov[1];
    int n;
    int newfd;
    union{
    struct cmsghdr cm;
    char control[CMSG_SPACE(sizeof(int))];
    }control_un;
    struct cmsghdr *cmptr;
    msg.msg_control = control_un.control;
    msg.msg_controllen = sizeof(control_un.control);

    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    iov[0].iov_base = ptr;
    iov[0].iov_len = nbytes;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;
    if((n = recvmsg(fd, &msg, 0)) <= 0)
    {
        return n;
    }
    cmptr = CMSG_FIRSTHDR(&msg);
    if((cmptr != NULL) && (cmptr->cmsg_len == CMSG_LEN(sizeof(int))))
    {
        if(cmptr->cmsg_level != SOL_SOCKET)
        {
            return (-1);
        }
        if(cmptr->cmsg_type != SCM_RIGHTS)
        {
            return (-1);
        }
        *recvfd = *((int*)CMSG_DATA(cmptr));
    }
    else
    {
        if(cmptr == NULL)
        *recvfd = -1; // descriptor was not passed
    }
    return n;
}
