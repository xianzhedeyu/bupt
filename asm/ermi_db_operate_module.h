#include <unistd.h>
#include <sqlite3.h>
#include<stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include "ermi.h"

#ifndef MAX_ROW
#define MAX_ROW 64
#define MAX_STRING 256
#define MAX_IP 16
#define DB_NAME  "/home/orion/workspaces/c/erm/ERMDB/test.db"
#endif



typedef struct _eqaminfo{
	char Qam_name[MAX_STRING];                  		//Qam名字             
	char Qam_group_name[MAX_STRING];        		//Qam group名字                           
	int totalbw;                   			//总带宽
	int cost;							//代价
	int Frequency;						//调制频率
	char Modmode[MAX_STRING];						//调制模式
	int tsid;							//该QAM在PAT表中用的TSID
	short Interleaver;					//由N6建立的描述FEC interleaver的常量
	short Annex;						//描述QAM ITU-T annex的由N6建立的常量http://zhidao.baidu.com/question/222860599.html
	int Channelwidth;						//QAM的Channel Width
	int Valid;								//Qam设备的有效性
	char Qam_ip[16];						//Qam设备管理IP地址

}eqaminfo;

typedef struct _eqamnextserver                      //
{

	char Qam_name[MAX_STRING];  					//Qam名字

	char Streaming_zone_name[MAX_STRING];			//流域名
	char Next_server_add[MAX_STRING];				//主下一跳服务地址

	int Next_server_add_standby_numb;		//备用下一跳服务个数
	char Next_server_add_standby[MAX_ROW][MAX_STRING];		//备用下一跳服务数组
}eqamnextserver;

typedef struct _eqamudp
{
	char Qam_name[MAX_STRING];  					//Qam名字 

	int udp_numb;							//UDP个数
	int Udpport_ProgramID[MAX_STRING][2];		//UDP信息及对应节目号

	int availablebw;							//无用0代表,可用带宽值
}eqamudp;

typedef struct 	_eqaminput
{
	char Qam_name[MAX_STRING];  					//Qam名字 
	int input_numb;							//输入地址信息个数

	char SubnetMask[MAX_ROW][MAX_STRING];
	char Input_Host[MAX_ROW][MAX_STRING];
	int PortID[MAX_ROW][3];
	int MaxGroupBW[MAX_ROW];
	char GroupName[MAX_ROW][MAX_STRING];
}eqaminput;

typedef struct _eqamselectinfo
{
 char qam_name[MAX_STRING]; 	 //in
char qam_add[MAX_IP];			 //out
 int available_bw;							//out
 int udp_num;					//out
 int udp_program[MAX_STRING][2];	//out
 char input_add[MAX_IP];			//out SS addr
 int input_port;					//out SS port
 char next_add[MAX_IP];			//out QAM addr
 int next_port;					//out QAM port
  int Frequency;							//out 调制频率
  char qam_group[MAX_STRING];
  char input_group[MAX_STRING];			//out
  char Modmode[MAX_STRING];				//out
}eqamselectinfo;

typedef struct _eqamsdinfo
{
char qam_name[MAX_STRING];
int udp_port;
int udp_state;	//setup pre:1    setup ok:2   teardown pre:3  teardown ok:4
int use_bw;
char qam_session[MAX_STRING];
}eqamsdinfo;

typedef struct _eqamselectinfo_down
{
 char qam_name[MAX_STRING];
 int use_bw;
 char qam_session[MAX_STRING];				//out
 char qam_add[MAX_IP];			 //out
 char next_add[MAX_IP];			//out
 int next_port;					//out
}eqamselectinfo_down;

int EDB_exect(sqlite3 *db,const char * sql,const int sql_type,const char * tablename);
int EDB_create();
int EDB_initializtion();
int EQAM_ADD(int infocheck, const eqaminfo infocontent, int nextservercheck, const eqamnextserver  nextservercontent ,
	int udpcheck, const eqamudp udpcontent,int inputcheck,const eqaminput inputcontent);
int EQAM_SETUP_DOWN(const eqamsdinfo &qma_sd);
int EQAM_SELECT(eqamselectinfo *qam_info,int qam_numb);
int EQAM_DOWN_SELECT(eqamselectinfo_down *qs);
int EQAM_SESSION_SELECT(EGETPARAM_MSG getparam, string &res);
void qamsort(eqamselectinfo s[], int l, int r);
