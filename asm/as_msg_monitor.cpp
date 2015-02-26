#include "as_msg_monitor.h"
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

pthread_mutex_t mutex;
List list;
void* parse(void* argument) 
{
    Argument element = *(Argument*)argument;
    ASNode asnode;
    char keepalive[3];
    
    while(1)
    {
        char str[1024];
        memset(str, 0x00, 1024);
        int n = 0;
        if((n = read(element.connfd, str, 1024)) > 0)
        {
            byte* VREP = (byte *)str;
	        switch (VREP[2]) {
	            case VREP_ID_D6_OPEN:
	            	OPEN M_OPEN;
	            	if (ParseOPEN(VREP, M_OPEN)) {
	            		OPENOut(M_OPEN);
	            		InspectOPEN(M_OPEN);
	            	}
                    //新建一个节点
                    asnode.cpuload = 0;
                    asnode.memload = 0;
                    asnode.weight = 0;
                    strcpy(asnode.ip, element.ip);
                    pthread_mutex_lock(&mutex);
                    insert(list, asnode);
                    pthread_mutex_unlock(&mutex);
                    //PackageKeepalive(keepalive);
                    //write(element.connfd, keepalive, strlen(keepalive) + 1); 
	            	//将M_OPEN写入数据库
	            	break;
	            case VREP_ID_D6_UPDATE:

            printf("###################\n");
	            	UPDATE M_UPDATE;
	            	if (ParseUPDATE(VREP, M_UPDATE)) {
	            		UPDATEOut(M_UPDATE);
	            		InspectUPDATE(M_UPDATE);
	            	}
                    asnode.cpuload = M_UPDATE.cpu;
                    asnode.memload = M_UPDATE.memory;
                    asnode.weight = (asnode.cpuload + asnode.memload) / 2;
                    strcpy(asnode.ip, element.ip);
                    pthread_mutex_lock(&mutex);
                    update(list, asnode, element.ip);
                    pthread_mutex_unlock(&mutex);
                    PackageKeepalive(keepalive);
                    write(element.connfd, keepalive, strlen(keepalive) + 1);
                    break;
	            case VREP_ID_D6_NOTIFICATION:
	            	cout << "MessageType is: NOTIFICATION" << endl;
	            	ParseNOTIFICATION(VREP);
	            	//上报错误
	            	break;
	            case VREP_ID_D6_KEEPALIVE:
	            	//发送KEEPALIVE消息
	            	cout << "MessageType is: KEEPALIVE" << endl;
	            	break;
	            default:
	            	//TODO:错误处理
	            	cout << "MessageType ERROR!" << endl;
	            	cout << (short) VREP[2] << endl;
	            	break;
	        }
        }
    }
}
void as_msg_process() {
    int listenfd, connfd;

    pthread_mutex_init(&mutex, NULL);
    init(list);

    listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in servaddr;
    struct sockaddr_in cliaddr;
    socklen_t clilen;

    memset(&servaddr, 0x00, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(VREP_SERVER);
    bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    listen(listenfd, 5);
    while(1) 
    {
        clilen = sizeof(cliaddr);
        connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &clilen);
        Argument argument;
        argument.connfd = connfd;
        char *ip;
        ip = inet_ntoa(cliaddr.sin_addr);
        strcpy(argument.ip, ip);
        pthread_t thread_id;
        pthread_create(&thread_id, NULL, &parse, &argument);
    }
}
