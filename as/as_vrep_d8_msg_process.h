#include <iostream>
#define btos(b,p) (short)b[p+1]|(short)b[p]<<8;        //双字节转short
#define btoi(b,p) (int)b[p+1]|(int)b[p]<<8;            //双字节转int
using namespace std;

typedef unsigned char byte;
typedef struct _Parameter                    //OPEN消息参数结构
{
	short ParameterType;                           //参数类型
	short SubParametersLen;                        //参数长度
	byte ParameterValue[256];                      //参数值(Type为1时其值为该参数在Capability数组中的位置)
}Parameter;
typedef struct _Capability                    //OPEN消息中的参数Capability Information
{
	short M_capabilityCode;                        //功能代号                    
	short M_capabilityLen;                         //该功能值的长度
	byte M_capability[4];                           
}Capability;
typedef struct _OPEN                          //OPEN消息
{
	short Len;                                     //消息长度(包括自己)
	byte type;                                     //消息类别
	short version;                                 //协议版本
	short holdtime;                                //ED申请的HoldTime值
	byte ID[4];                                      //VREP标识(如果N接口没有配则设为设备的IPv4地址)
	short parametersLen;                           //消息所带参数的总长度
	Parameter parameters[256];                        //参数队列
	int ParameterNum;                              //参数个数
	Capability capability[256];                        //功能(参数的一种)队列
}OPEN;

typedef struct _Attributes                    //UPDATE消息属性结构
{
	byte AttrFlag;                                 //属性标志(高位比特为0表示已知属性，1表示未知属性，其他位无效)
	byte AttrType;                                 //属性代号
	short AttrLen;                                 //属性值的长度
	byte* AttrValue;                               //属性值
}Attributes;
typedef struct _UPDATE                      //UPDATE消息
{
	short Len;                                     //消息长度
	byte type;                                     //消息类型
	Attributes attributes[256];                    //参数指针(指向所有参数的字符串，无实际意义)
	int AttributeNum;                              //参数个数
	int ServiceStatus;                             //服务状态
    double cpu;                                    //cpu负载
    double memory;                                 //内存负载
}UPDATE;

//static int btoi(byte* b);                //字节数组(4位)转化为int型
//static short btos(byte* b);
bool ParseOPEN(byte* VREP,OPEN &M_OPEN);                      //OPEN消息解析函数
void InspectOPEN(OPEN &M_OPEN);                            //OPEN检查
void HandlUnkownAttr(Attributes &attr);                     //未知参数处理
void ParseServiceStatus(byte* Attr,int &ServiceStatus);          //解析参数ServiceStatus
bool ParseAttr(Attributes* &M_attr,int ANum,UPDATE &M_UPDATE);        //解析参数函数
bool ParseUPDATE(byte VREP[],UPDATE &M_UPDATE);                    //UPDATE消息解析函数
void InspectUPDATE(UPDATE &M_UPDATE);                        //UPDATE检查
bool ParseNOTIFICATION(byte VREP[]);                          //NOTIFICATION消息解析函数
bool Parse(byte VREP[]);                       //解析函数入口
bool PackageOpen(OPEN M_OPEN,char* OpenMessage);                //打包OPEN消息
bool PackageKeepalive(char* KeepMessage);                       //打包KEEPALIVE消息
bool PackageNotification(short ECode,short ESubCode,char* Date,char* NotificationMessage);        //打包NOTIFICATION消息
bool PackageUpdate(UPDATE M_UPDATE, char *UpdateMessage);
void OPENOut(OPEN M_OPEN);                                    //输出OPEN消息
void UPDATEOut(UPDATE M_UPDATE);                              //输出UPDATE消息
