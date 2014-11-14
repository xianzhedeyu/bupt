#include <iostream>
#define btos(b,p) (short)b[p+1]|(short)b[p]<<8;
#define btoi(b,p) (int)b[p+1]|(int)b[p]<<8;
using namespace std;

typedef unsigned char byte;
typedef struct _Parameters
{
	short ParameterType;
	short SubParametersLen;
	byte ParameterValue[256];
}drrp_parameters;
typedef struct _Capability
{
	short MSG_capabilityCode;
	short MSG_capabilityLen;
	byte MSG_capability[4];
}Capability;
typedef struct _RouteTypesSupported
{
	int AddressFamily;
	int ApplicationProtocol;
}RouteTypesSupported;
typedef struct _OPEN
{
	short Len;
	byte type;
	short version;
	short holdtime;
	byte AddressDomain[4];
	byte ID[4];
	short parametersLen;
	drrp_parameters parameters[256];
	int ParameterNum;
	Capability capability[256];
	RouteTypesSupported routetypessupported[256];
}OPEN;

typedef struct _Attributes
{
	byte AttrFlag;
	byte AttrType;
	short AttrLen;
	byte* AttrValue;
}Attributes;
typedef struct _Routes
{
	int AddFamily;
	int AppProtocol;
	short AddLen;
	byte Address[256];
}Routes;
typedef struct _DOCSISCapability
{
	unsigned int Interleaver;
	unsigned int DOCSISMode;
	short Modulation;
	short ChannelWidth;
	short J83;
}DOCSISCapability;
typedef struct _QAMChannelConfig
{
	unsigned int Frequency;
	byte Modulation;
	byte J83_ChannelWidth;
	byte I;
	byte J;
}QAMChannelConfig;
typedef struct _DEPIControlAddr
{
	byte NetworkPrefix[4];
	short Length;
	byte Host[256];//FQDN
}DEPIControlAddr;
typedef struct _Server
{
	unsigned int NextHopAddressDomain;
	short Length;
	byte Server[256];
}Server;
typedef struct _UPDATE
{
	short Len;
	byte type;
	Attributes attributes[256];
	int AttributeNum;
	Server NextHopServer;
	Routes	WithdrawnRoutes[256];
	int WRoutesNum;
	Routes ReachableRoutes[256];
	int RRoutesNum;
	int totalbw;
	short PortID;
	int ServiceStatus;
	short QAMID;
	DOCSISCapability DocsisCapability;
	QAMChannelConfig QamChannelConfig;	
	DEPIControlAddr DepiControlAddr;
}UPDATE;
static int ermibytes2int(byte* b);
bool parse_drrp_open(byte* DRRP,OPEN &MSG_OPEN);
void ins_drrp_open(OPEN &MSG_OPEN);
void hdl_undrrp_attr(Attributes &attr);
void parse_drrp_route(byte* Attr,short Len,Routes* routes,int &Num);
void parse_drrp_nhs(byte* Attr,short Len,Server &MSG_NextHopServer);
bool parse_drrp_attr(Attributes* &MSG_attr,int ANum,UPDATE &MSG_UPDATE);
bool parse_drrp_update(byte DRRP[],UPDATE &MSG_UPDATE);
void ins_drrp_update(UPDATE &MSG_UPDATE);
bool parse_drrp_notification(byte DRRP[]);
bool drrp_parse(byte DRRP[]);
bool pack_drrp_open(OPEN MSG_OPEN,char* OpenMessage);
bool pack_drrp_keepalive(char* KeepMessage);
bool pack_drrp_notification(short ECode,short ESubCode,char* Date,char* NotificationMessage);
void print_drrp_open(OPEN MSG_OPEN);
void print_drrp_update(UPDATE MSG_UPDATE);
