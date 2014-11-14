#include <iostream>
#define btos(b,p) (short)b[p+1]|(short)b[p]<<8;        //双字节转short
#define btoi(b,p) (int)b[p+1]|(int)b[p]<<8;            //双字节转int
using namespace std;
/*
typedef enum MessageType
{
	OPEN=1,
	UPDATE,
	NOTIFICATION,
	KEEPALIVE
}Type;
*/
typedef unsigned char byte;
typedef struct _Parameters                    //OPEN消息参数结构
{
	short ParameterType;                           //参数类型
	short SubParametersLen;                        //参数长度
	byte ParameterValue[256];                      //参数值(Type为1时其值为该参数在Capability数组中的位置)
}Parameters;
typedef struct _Capability                    //OPEN消息中的参数Capability Information
{
	short M_capabilityCode;                        //功能代号                    
	short M_capabilityLen;                         //该功能值的长度
	byte M_capability[4];                            //功能值(Code为1时其值为对应路由类型数组的元素个数)
}Capability;
typedef struct _RouteTypesSupported           //OPEN消息中参数Capability Information中的RouteTypesSupported
{
	int AddressFamily;                           //该支持路由类型的地址族       
	int ApplicationProtocol;                     //应用协议                     
}RouteTypesSupported;
typedef struct _OPEN                          //OPEN消息
{
	short Len;                                     //消息长度(包括自己)
	byte type;                                     //消息类别
	short version;                                 //协议版本
	short holdtime;                                //ED申请的HoldTime值
	byte ID[4];                                      //VREP标识(如果N接口没有配则设为设备的IPv4地址)
	short parametersLen;                           //消息所带参数的总长度
	Parameters parameters[256];                        //参数队列
	int ParameterNum;                              //参数个数
	Capability capability[256];                        //功能(参数的一种)队列
	RouteTypesSupported routetypessupported[256];      //支持的路由类型(功能的一种)队列
}OPEN;


typedef struct _PortID
{
        short slotnumber;
        short portnumber;
        short subinterface;
}PortID;
typedef struct _Attributes                    //UPDATE消息属性结构
{
	byte AttrFlag;                                 //属性标志(高位比特为0表示已知属性，1表示未知属性，其他位无效)
	byte AttrType;                                 //属性代号
	short AttrLen;                                 //属性值的长度
	byte* AttrValue;                               //属性值(值为该位置的字符串，可能无实际意义)
}Attributes;
typedef struct _Routes                       //UPDATE消息路由结构
{
	int AddFamily;                               //路由地址族                   
	int AppProtocol;                             //应用协议                     
	short AddLen;                                  //地址长度
	byte Address[256];                                 //地址值
}Routes;
typedef struct _Server                       //UPDATE消息主机结构(描述下一跳地址信息)
{
	short AddLen;                                  //地址长度
	byte Address[256];                                 //地址值host[":"port]
	short ZoneNameLen;                             //Streaming Zone名的长度
	byte ZoneName[256];                                //Streaming Zone名
}Server;
typedef struct _Input                        //UPDATE消息边缘输入属性结构(用于描述推流的目的地址)
{
	byte SubnetMask[256];                              //子网掩码
	byte Host[256];                                    //主机地址(主机名或RFC1123规定的IPv4格式地址或RFC2373规定的IPv6格式地址，
																		//且IPv6地址要用"[]"括起来)
	PortID portID;                                    //物理端口号
	int MaxGroupBW;                                //边缘输入可承载的最大组带宽(Kbps)
	byte GroupName[256];                               //该输入对应的Edge Input Group名
}Input;
typedef struct _QAMParameters               //UPDATE消息QAM参数
{
	int Frequency;                                 //调制频率
	byte Modmode[256];                             //调制模式
	byte Interleaver;                              //由N6建立的描述FEC interleaver的常量
	short TSID;                                    //该QAM在PAT表中用的TSID
	byte Annex;                                    //描述QAM ITU-T annex的由N6建立的常量
	short Channelwidth;                            //QAM的Channel Width
}QAMParameters;
typedef struct _StaticPorts                 //UPDATE消息UDP MAP表中的静态端口
{
	short UDPPort;                                 //UDP端口
	short ProgramID;                               //端口对应的节目号
}StaticPorts;
typedef struct _DynamicPorts                //UPDATE消息UDP MAP表中的动态端口
{
	short StartingPort;                            //动态端口范围的起始端口
	short StartingPID;                             //起始端口对应的节目号
	int Count;                                     //动态端口的范围(端口和节目号递增加一)
}DynamicPorts;
typedef struct _NHSAlternates               //UPDATE消息NextHopServerAlternates参数
{
	short NumAlternates;                           //alternate servers的个数
	byte server[256][256];                         //一串alternate servers
}NHSAlternates;
typedef struct _UPDATE                      //UPDATE消息
{
	short Len;                                     //消息长度
	byte type;                                     //消息类型
	Attributes attributes[256];                    //参数指针(指向所有参数的字符串，无实际意义)
	int AttributeNum;                              //参数个数
	Routes	WithdrawnRoutes[256];                  //不可达路由串
	int WRoutesNum;                                //不可达路由个数
	Routes ReachableRoutes[256];                   //可达路由串
	int RRoutesNum;                                //可达路由个数
	Server  NextHopServer;                         //下一跳服务器
	byte  QAMName[256][256];                       //QAM名串
	int QAMNameNum;                                //QAM名个数
	int totalbw;                                   //总带宽
	int availablebw;                               //可用带宽
	short cost;                                    //cost
	Input EdgeInput[256];                          //边缘输入串
	int EdgeInputNum;
	QAMParameters QAMP;                            //QAM参数
	StaticPorts SPorts[256];                       //UDP MAP中的静态端口串
	int SPortsNum;                                 //静态端口个数
	DynamicPorts DPorts[256];                      //UDP MAP中的动态端口串
	int DPortsNum;                                 //动态端口个数
	int ServiceStatus;                             //服务状态
	int MaxMpegFlows;                              //支持的最大并行流个数
	NHSAlternates Alternates;                      //alternate servers
	PortID OutputPort;                             //QAM被分配的端口
}UPDATE;
static int bytes2int(byte* b);                //字节数组(4位)转化为int型
bool ParseOPEN(byte* VREP,OPEN &M_OPEN);                      //OPEN消息解析函数
void InspectOPEN(OPEN &M_OPEN);                            //OPEN检查
void HandlUnkownAttr(Attributes &attr);                     //未知参数处理
void ParseRoutes(byte* Attr,short Len,Routes* routes,int &Num);           //解析参数WithdrawnRoutes和ReachableRoutes
void ParseNHS(byte* Attr,short Len,Server &M_NextHopServer);        //解析参数NextHopServer
void ParseQAMN(byte* Attr,short Len,byte QAMNames[256][256],int &Num);       //解析参数QAMNames
void ParseEdgeInput(byte* Attr,short Len,Input* M_EdgeInput,int &Num);    //解析参数EdgeInput
void ParseQAMP(byte* Attr,short Len,QAMParameters &M_QAMP);         //解析参数QAMParameters
void ParseServiceStatus(byte* Attr,int &ServiceStatus);          //解析参数ServiceStatus
void ParseUDPMap(byte* Attr,short Len,StaticPorts* SPorts,DynamicPorts* DPorts,int &SNum,int &DNum);     //解析参数UDPMap
void ParseNHSA(byte* Attr,short Len,NHSAlternates &M_Alternates);        //解析参数NextHopServerAlternates
void ParsePortID(byte *Port,PortID &M_Port);                        //解析PortID
bool ParseAttr(Attributes* &M_attr,int ANum,UPDATE &M_UPDATE);        //解析参数函数
bool ParseUPDATE(byte VREP[],UPDATE &M_UPDATE);                    //UPDATE消息解析函数
void InspectUPDATE(UPDATE &M_UPDATE);                        //UPDATE检查
bool ParseNOTIFICATION(byte VREP[]);                          //NOTIFICATION消息解析函数
bool Parse(byte VREP[]);                       //解析函数入口
bool PackageOpen(OPEN M_OPEN,char* OpenMessage);                //打包OPEN消息
bool PackageKeepalive(char* KeepMessage);                       //打包KEEPALIVE消息
bool PackageNotification(short ECode,short ESubCode,char* Date,char* NotificationMessage);        //打包NOTIFICATION消息
void OPENOut(OPEN M_OPEN);                                    //输出OPEN消息
void UPDATEOut(UPDATE M_UPDATE);                              //输出UPDATE消息
