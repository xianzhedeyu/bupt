#ifndef _SM_QUEUEMANAGER_H_
#define  _SM_QUEUEMANAGER_H_

//#define SM_MAXNUM 100

//#pragma pack(1)
/*共享内存区的数据结构*/
typedef struct
{
	int data_flag;/*有无数据*/
	int process_flag;/*进程标志*/
	int data_len;
	char data[1024];
}SOCKET_DATA;

int PushToQueue(SOCKET_DATA* pItem,SOCKET_DATA*);

int PopFromQueue(int ,SOCKET_DATA* pItem,SOCKET_DATA*);

#endif  //_erm_QUEUEMANAGER_H_

