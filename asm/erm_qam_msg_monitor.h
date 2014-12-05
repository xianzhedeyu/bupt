
#include"erm_vrep_d6_msg_process.h"


typedef struct _stQAM_CONNECT_INFO{
	pthread_t nTID;
	int nsock[100];
	int nNUM; //一个线程内的qam数
	int nflag;


}stQAM_CONNECT_INFO;

typedef struct _stQAM_FIRST_UPDATA_INFO{
	int nsd;
	int nflag;
	short cost;                                    //cost
	int ServiceStatus;                             //服务状态
	int EdgeInputNum;
	Input EdgeInput[256];                          //边缘输入串
	
}stQAM_FIRST_UPDATA_INFO;


#define MAXEPOLLSIZE 10000
#define MAXBUF 4096

typedef struct _stQAM_STATE_CHECK{

int sockfd;
time_t Start_time;
char szQamname[];

}stQAM_STATE_CHECK;

void qam_msg_process();
