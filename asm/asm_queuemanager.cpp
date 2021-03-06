#include"asm_task_control_module.h"

extern SOCKET_DATA S_pItem[ERM_MAXNUM];
extern pthread_queue data[ERM_MAXNUM];

int PushToQueue(SOCKET_DATA* w_data,SOCKET_DATA* mmap)
{
	memset(S_pItem,0x00,sizeof(S_pItem));
	memcpy(S_pItem,mmap,sizeof(S_pItem));
	int num=0;
	//遍历这个队列中所有的数据块
	for(num=0;num<ERM_MAXNUM;num++){
		//查找未用的数据块，找到就将数据放到此数据块中
		if(S_pItem[num].data_flag==0){
			S_pItem[num].data_flag=1;
			S_pItem[num].data_len=w_data->data_len;
			memcpy(S_pItem[num].data,w_data->data,w_data->data_len);
			break;
		}
	}
	//memcpy(mmap,S_pItem,sizeof(S_pItem));
	memcpy(mmap,S_pItem,sizeof(S_pItem));
	//if find it, return the num of the data block
	if(num<ERM_MAXNUM)
		return num;
	//not find
	return -1;
}

int PopFromQueue(int num,SOCKET_DATA* r_data,SOCKET_DATA * mmap)
{
	
	int i,id=0;
	memset(S_pItem,0x00,sizeof(S_pItem));
	memcpy(S_pItem,mmap,sizeof(S_pItem));

	for(i=0;i<ERM_MAXNUM;i++){
		if(data[i].pthread_flag==1&&data[i].data_flag==0){
			id=data[i].data_id;
			memset(&S_pItem[id],0x00,sizeof(S_pItem[id]));
			data[i].pthread_flag=0;
			data[i].data_id=0;
		}
	}


	//fprintf(stderr,"@@@@@@@@@@@@@@@%d   %s\n",num,S_pItem[num].data);
	if(S_pItem[num].data_flag==1){
		S_pItem[num].data_flag=2;///??????????????????????!!!!!!!!!!!!!!!!!!!
		r_data->data_len=S_pItem[num].data_len;
		memcpy(r_data->data,S_pItem[num].data,r_data->data_len);
	}else{
		num=-1;
	}
	memcpy(mmap,S_pItem,sizeof(S_pItem));

	return num;
}
int FreeQueue(SOCKET_DATA* r_data,SOCKET_DATA * mmap)
{
	
}
