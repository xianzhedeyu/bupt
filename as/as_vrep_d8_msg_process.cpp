#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include "as_vrep_d8_msg_process.h"
#include "public_def.h"

static int bytes2int(byte* b) //字节数组(4位)转化为int型,整型的低字节位是字节数组的高位
{
	int mask = 0xff;
	int temp = 0;
	int res = 0;
	for (int i = 0; i < 4; i++) {
		res <<= 8;
		temp = b[i] & mask;
		res |= temp;
	}
	return res;
}
static unsigned int bytes2int2(byte* b) //字节数组(4位)转化为int型,整型的高字节位是字节数组的高位
{
	unsigned int mask = 0xff;
	unsigned int temp = 0;
	unsigned int res = 0;
	for (int i = 0; i < 4; i++) {
		res <<= 8;
		temp = b[3 - i] & mask;
		res |= temp;
	}
	return res;
}
// 将iSource转为byte数组，字节数组的高位是整型的低字节位
static void toByteArray(int iSource, byte* Arr) {
	for (int i = 0; (i < 4); i++) {
		Arr[3 - i] = (byte) (iSource >> 8 * i & 0xFF);
	}
}
bool ParseOPEN(byte* VREP, OPEN &M_OPEN) //OPEN消息解析函数
{
	short M_Len = 0; //消息长度
	short M_version = 0; //协议版本
	short M_holdtime = 0; //HoldTime
	short M_parametersLen = 0; //参数


	int i = 0; //计数用

	M_Len = btos(VREP,0);
	fprintf(stderr, "M_len:%d\n", M_Len);
	M_version = (short) VREP[3];
	fprintf(stderr, "M_version:%d\n", M_version);
	M_holdtime = btos(VREP,5);
	fprintf(stderr, "M_holdtime:%d\n", M_holdtime);
	//...
	byte* ID = VREP + 11;
	for (i = 0; i < 4; i++)
		M_OPEN.ID[i] = ID[i];
	M_parametersLen = btos(VREP,15);
	fprintf(stderr, "M_parametersLen:%d\n", M_parametersLen);

	short restPlen = M_parametersLen; //用于计算剩余参数长度
	short restRTlen = 0; //用于计算Capability为RouteTypes时剩余capabilityLen
	int PNum = 0; // 参数个数
	int CNum = 0; //Capability个数
	int RTNum = 0; //RouteTypesSupported个数
	short len = 0; //取完的参数长度之和
	while (restPlen > 0) {
		int begin = 17 + 4 * PNum + len; //起点指针
		M_OPEN.parameters[PNum].ParameterType = btos(VREP,begin);
		fprintf(stderr, "ParameterType:%d\n", M_OPEN.parameters[PNum].ParameterType);
		M_OPEN.parameters[PNum].SubParametersLen = btos(VREP,begin+2);
		fprintf(stderr, "SubParametersLen:%d\n", M_OPEN.parameters[PNum].SubParametersLen);
		if (M_OPEN.parameters[PNum].ParameterType == VREP_ID_D6_OPEN_CapabilityInformation) //若为Capability Information参数
		{
			M_OPEN.capability[CNum].M_capabilityCode = btos(VREP,begin+4);
			fprintf(stderr, "M_capabilityCode:%d\n", M_OPEN.capability[CNum].M_capabilityCode);
			M_OPEN.capability[CNum].M_capabilityLen = btos(VREP,begin+6);
			fprintf(stderr, "M_capabilityLen:%d\n", M_OPEN.capability[CNum].M_capabilityLen);
            if (M_OPEN.capability[CNum].M_capabilityCode == VREP_ID_D6_OPEN_CapabilityInformation_SendReceiveCapability) //Send Receive Capability
			{
				byte* SendReceiveCapability;
				SendReceiveCapability = VREP + begin + 8;
				M_OPEN.capability[CNum].M_capability[0] = SendReceiveCapability[0];
				M_OPEN.capability[CNum].M_capability[1] = SendReceiveCapability[1];
				M_OPEN.capability[CNum].M_capability[2] = SendReceiveCapability[2];
				M_OPEN.capability[CNum].M_capability[3] = SendReceiveCapability[3]; //4字节
			}
            //else if(M_OPEN.capability[CNum].M_capabilityCode == VREP_ID_D8_OPEN_CapabilityInformation_Memory)
            //{
            //    byte* Memory;
            //    Memory = VREP + begin + 8;
            //    M_OPEN.capability[CNum].M_capability[0] = Memory[0];
            //    M_OPEN.capability[CNum].M_capability[1] = Memory[1];
            //    M_OPEN.capability[CNum].M_capability[2] = Memory[2];
            //    M_OPEN.capability[CNum].M_capability[3] = Memory[3];
            //}
            //else if(M_OPEN.capability[CNum].M_capabilityCode == VREP_ID_D8_OPEN_CapabilityInformation_CPUS) 
            //{
            //    byte* cpus;
            //    cpus = VREP + begin + 8;
            //    M_OPEN.capability[CNum].M_capability[0] = cpus[0];
            //    M_OPEN.capability[CNum].M_capability[1] = cpus[1];
            //    M_OPEN.capability[CNum].M_capability[2] = cpus[2];
            //    M_OPEN.capability[CNum].M_capability[3] = cpus[3];
            //}
			//	byte* CI=(byte*)"Capability Information";
			toByteArray(CNum, M_OPEN.parameters[PNum].ParameterValue); //此处值为该Capability所在数组的位置
			CNum++;
		} 
        else {
			byte* ParameterValue = VREP + begin + 4;
			//	byte M_ParameterValue[256];
			for (i = 0; i < M_OPEN.parameters[PNum].SubParametersLen; i++) {
				M_OPEN.parameters[PNum].ParameterValue[i] = ParameterValue[i];
				fprintf(stderr, "ParameterValue:%d\n", M_OPEN.parameters[PNum].ParameterValue[i]);
			}
			//	M_OPEN.parameters[PNum].ParameterValue=M_ParameterValue;
		}
		len += M_OPEN.parameters[PNum].SubParametersLen;
		restPlen = restPlen - (4 + M_OPEN.parameters[PNum].SubParametersLen);
		PNum++;
	}

	//写入结构
	M_OPEN.Len = M_Len;
	M_OPEN.type = '1';
	M_OPEN.version = M_version;
	M_OPEN.holdtime = M_holdtime;
	M_OPEN.parametersLen = M_parametersLen;
	M_OPEN.ParameterNum = PNum;

	return true;
}
void InspectOPEN(OPEN &M_OPEN) //OPEN消息检查
{
	int i = 0;
	if (M_OPEN.Len < 17)
		cout << "The OPEN Message Length is too short!" << M_OPEN.Len << endl;
	for (i = 0; i < M_OPEN.ParameterNum; i++) {
		if (M_OPEN.parameters[i].ParameterType != 2)
			continue;
		else
			break;
	}
	if (i == M_OPEN.ParameterNum)
		cout << "There is a lack of Parameter 'StreamingZone' in this OPEN Message" << endl;
	for (i = 0; i < M_OPEN.ParameterNum; i++) {
		if (M_OPEN.parameters[i].ParameterType != 3)
			continue;
		else
			break;
	}
	if (i == M_OPEN.ParameterNum)
		cout << "There is a lack of Parameter 'Component Name' in this OPEN Message" << endl;
}

void InspectUPDATE(UPDATE &M_UPDATE)
{
}

//void ParseNHS(byte* Attr, short Len, Server &M_NextHopServer) //解析参数NextHopServer
//{
//	M_NextHopServer.AddLen = btos(Attr,4);//开始是有四个保留字节
//	int i = 0;
//	byte* M_Address = Attr + 6;
//	for (i = 0; i < M_NextHopServer.AddLen; i++) {
//		M_NextHopServer.Address[i] = M_Address[i];
//	}
//
//	M_NextHopServer.ZoneNameLen = btos(Attr,M_NextHopServer.AddLen+6);
//	byte* M_ZoneName = Attr + M_NextHopServer.AddLen + 8;
//	for (i = 0; i < M_NextHopServer.ZoneNameLen; i++) {
//		M_NextHopServer.ZoneName[i] = M_ZoneName[i];
//	}
//
//}
//void ParseServiceStatus(byte* Attr, int &ServiceStatus) //解析参数ServiceStatus
//{
//	byte M_ServiceStatus[4];
//	for (int j = 0; j < 4; j++) {
//		M_ServiceStatus[j] = Attr[j];
//	}
//	ServiceStatus = bytes2int(M_ServiceStatus);
//}
//void ParseNHSA(byte* Attr, short Len, NHSAlternates &M_Alternates) //解析参数NextHopServerAlternates
//{
//	M_Alternates.NumAlternates = btos(Attr,0);
//	int ServerLen = 0;
//	short sumlen = 2;
//	for (int i = 0; i < M_Alternates.NumAlternates; i++) {
//		ServerLen = btos(Attr,sumlen);
//		byte* M_server = Attr + sumlen + 2;
//		for (int j = 0; j < ServerLen; j++) {
//			M_Alternates.server[i][j] = M_server[j];
//		}
//		sumlen = sumlen + 2 + ServerLen;
//	}
//}
void HandlUnkownAttr(Attributes &attr, UPDATE &M_UPDATE) //未知参数处理
{
    if((unsigned int)attr.AttrType == 247) {
        M_UPDATE.memory = *(double*)attr.AttrValue;
    }
    if((unsigned int)attr.AttrType == 248) {
        M_UPDATE.cpu = *(double*)attr.AttrValue;
    }
}
bool ParseAttr(Attributes* M_attr, int ANum, UPDATE &M_UPDATE) //参数解析函数
{
	for (int i = 0; i < ANum; i++) {
		if (M_attr[i].AttrFlag >> 7)//只使用了这个字节中的最高位
		{
			HandlUnkownAttr(M_attr[i], M_UPDATE); //未知参数处理
		}
		fprintf(stderr, "type:%d\n", M_attr[i].AttrType);
	    //ParseServiceStatus(M_attr[i].AttrValue, M_UPDATE.ServiceStatus); //解析参数ServiceStatus
	}
	return true;
}
bool ParseUPDATE(byte VREP[], UPDATE &M_UPDATE) //解析UPDATE消息函数
{
	short M_Len = 0;
	int ANum = 0;
	M_Len = btos(VREP,0);
	short restLen = M_Len - 3; //解析消息的剩余长度,初始为去掉头的长度
	cout << "reslen: " << restLen << endl;
	short len = 0; //取完的参数值长度之和
	while (restLen > 0) {
		int begin = 3 + 4 * ANum + len; //起点指针
		M_UPDATE.attributes[ANum].AttrFlag = VREP[begin];

		M_UPDATE.attributes[ANum].AttrType = VREP[begin + 1];
		M_UPDATE.attributes[ANum].AttrLen = btos(VREP,begin+2);
        M_UPDATE.attributes[ANum].AttrValue = (byte *)malloc(M_UPDATE.attributes[ANum].AttrLen);
        for(int i = 0; i < M_UPDATE.attributes[ANum].AttrLen; i++) {
		    M_UPDATE.attributes[ANum].AttrValue[i] = VREP[begin + 4 + i];
        }
		len += M_UPDATE.attributes[ANum].AttrLen;
		restLen = restLen - (4 + M_UPDATE.attributes[ANum].AttrLen);
		cout << "reslen: " << restLen << endl;
		ANum++;
	}

	//写入结构
	M_UPDATE.Len = M_Len;
	M_UPDATE.type = '2';
	M_UPDATE.AttributeNum = ANum;

	if (ParseAttr(M_UPDATE.attributes, ANum, M_UPDATE))
		return true;
	else
		return false;
}

bool ParseNOTIFICATION(byte VREP[]) {
	short M_Len = btos(VREP,0);
	short M_Code = (short) VREP[3];
	short M_Subcode = (short) VREP[4];
	short dateLen = M_Len - 5;
	byte M_Date[256];
	int i = 0;
	for (i = 0; i < dateLen; i++)
		M_Date[i] = VREP[5 + i];
	switch (M_Code) {
	case VREP_ID_D6_NOTIFICATION_MessageHeaderError: //Message Header Error
		if (M_Subcode == VREP_ID_D6_NOTIFICATION_MessageHeaderError_BadMessageLength) //Bad Message Length
			//TODO:错误处理
			cout << "Bad Message Length!" << endl;
		if (M_Subcode == VREP_ID_D6_NOTIFICATION_MessageHeaderError_BadMessageType) //Bad Message Type
			//TODO:错误处理
			cout << "Bad Message Type!" << endl;
		break;
	case VREP_ID_D6_NOTIFICATION_OPENMessageError: //OPEN Message Error
		if (M_Subcode == VREP_ID_D6_NOTIFICATION_OPENMessageError_UnsupportedVersionNumber) //Unsupported Version Number
			//TODO:错误处理
			cout << "Unsupported Version Number!" << endl;
		if (M_Subcode == VREP_ID_D6_NOTIFICATION_OPENMessageError_BadPeerAddressDomain) //Bad Peer Address Domain
			//TODO:错误处理
			cout << "Bad Peer Address Domain!" << endl;
		if (M_Subcode == VREP_ID_D6_NOTIFICATION_OPENMessageError_BadVREPIdentifier) //Bad VREP Identifier
			//TODO:错误处理
			cout << "Bad VREP Identifier!" << endl;
		if (M_Subcode == VREP_ID_D6_NOTIFICATION_OPENMessageError_UnsupportedOptionalParameter) //Unsupported Optional Parameter
			//TODO:错误处理
			cout << "Unsupported Optional Parameter!" << endl;
		if (M_Subcode == VREP_ID_D6_NOTIFICATION_OPENMessageError_UnacceptableHoldTime) //Unacceptable Hold Time
			//TODO:错误处理
			cout << "Unacceptable Hold Time!" << endl;
		if (M_Subcode == VREP_ID_D6_NOTIFICATION_OPENMessageError_UnsupportedCapability) //Unsupported Capability
			//TODO:错误处理
			cout << "Unsupported Capability!" << endl;
		if (M_Subcode == VREP_ID_D6_NOTIFICATION_OPENMessageError_CapabilityMismatch) //Capability Mismatch
			//TODO:错误处理
			cout << "Capability Mismatch!" << endl;
		break;
	case VREP_ID_D6_NOTIFICATION_UPDATEMessageError: //UPDATE Message Error
		if (M_Subcode == VREP_ID_D6_NOTIFICATION_UPDATEMessageError_MalformedAttributeList) //Malformed Attribute List
			//TODO:错误处理
			cout << "Malformed Attribute List!" << endl;
		if (M_Subcode == VREP_ID_D6_NOTIFICATION_UPDATEMessageError_nrecognizedWellknownAttribute) //Unrecognized Well-known Attribute
			//TODO:错误处理
			cout << "Unrecognized Well-known Attribute!" << endl;
		if (M_Subcode == VREP_ID_D6_NOTIFICATION_UPDATEMessageError_MissingWellknownMandatoryAttribute) //Missing Well-known Mandatory Attribute
			//TODO:错误处理
			cout << "Missing Well-known Mandatory Attribute!" << endl;
		if (M_Subcode == VREP_ID_D6_NOTIFICATION_UPDATEMessageError_AttributeFlagsError) //Attribute Flags Error
			//TODO:错误处理
			cout << "Attribute Flags Error!" << endl;
		if (M_Subcode == VREP_ID_D6_NOTIFICATION_UPDATEMessageError_AttributeLengthError) //Attribute Length Error
			//TODO:错误处理
			cout << "Attribute Length Error!" << endl;
		if (M_Subcode == VREP_ID_D6_NOTIFICATION_UPDATEMessageError_InvalidAttribute) //Invalid Attribute
			//TODO:错误处理
			cout << "Invalid Attribute!" << endl;
		break;
	case VREP_ID_D6_NOTIFICATION_HoldTimerExpired: //Hold Timer Expired
		//TODO:错误处理
		cout << "Hold Timer Expired!" << endl;
		break;
	case VREP_ID_D6_NOTIFICATION_FiniteStateMachineError: //Finite State Machine Error
		//TODO:错误处理
		cout << "Finite State Machine Error!" << endl;
		break;
	case VREP_ID_D6_NOTIFICATION_Cease: //Cease
		//TODO:错误处理
		cout << "Cease!" << endl;
		for (i = 0; i < dateLen; i++)
			cout << M_Date[i]; //上面应该也输出错误原因，暂不写
		break;
	default:
		break;

	}
	return true;
}
bool Parse(byte VREP[]) {
	switch (VREP[2]) {
	case VREP_ID_D6_OPEN:
		OPEN M_OPEN;
		if (ParseOPEN(VREP, M_OPEN)) {
			OPENOut(M_OPEN);
			InspectOPEN(M_OPEN);
		}
		//将M_OPEN写入数据库
		break;
	case VREP_ID_D6_UPDATE:
		UPDATE M_UPDATE;
		if (ParseUPDATE(VREP, M_UPDATE)) {
			UPDATEOut(M_UPDATE);
			InspectUPDATE(M_UPDATE);
		}
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
		return false;
		break;
	}

	return true;
}

bool PackageOpen(OPEN M_OPEN, char* OpenMessage) {
	OpenMessage[0] = (M_OPEN.Len >> 8) & 0xFF;//长度高字节
	OpenMessage[1] = M_OPEN.Len & 0xFF;//长度低字节
	OpenMessage[2] = 0x01;//OPEN类型
	OpenMessage[3] = 0x02;//版本
	OpenMessage[4]=0x00;//保留
	OpenMessage[5] = (M_OPEN.holdtime >> 8) & 0xFF;//holdtime高字节
	OpenMessage[6] = M_OPEN.holdtime & 0xFF;//holdtime低字节
	OpenMessage[7]=0x00;//保留
	OpenMessage[8]=0x00;//保留
	OpenMessage[9]=0x00;//保留
	OpenMessage[10]=0x00;//保留
	OpenMessage[11] = (char) M_OPEN.ID[0];//VREP ID
	OpenMessage[12] = (char) M_OPEN.ID[1];
	OpenMessage[13] = (char) M_OPEN.ID[2];
	OpenMessage[14] = (char) M_OPEN.ID[3];
	OpenMessage[15] = (M_OPEN.parametersLen >> 8) & 0xFF;//参数长度高字节
	OpenMessage[16] = M_OPEN.parametersLen & 0xFF;//参数长度低字节
	int sumlen = 0;
	for (int i = 0; i < M_OPEN.ParameterNum; i++) {
		OpenMessage[17 + sumlen] = (M_OPEN.parameters[i].ParameterType >> 8) & 0xFF;//参数类型高字节
		OpenMessage[18 + sumlen] = M_OPEN.parameters[i].ParameterType & 0xFF;//参数类型低字节
		OpenMessage[19 + sumlen] = (M_OPEN.parameters[i].SubParametersLen >> 8) & 0xFF;//参数长度高字节
		OpenMessage[20 + sumlen] = M_OPEN.parameters[i].SubParametersLen & 0xFF;//参数长度低字节
        int j;
		for (j = 0; j < M_OPEN.parameters[i].SubParametersLen; j++)
			OpenMessage[21 + sumlen + j] = (char) M_OPEN.parameters[i].ParameterValue[j]; //参数值
		sumlen = sumlen + 4 + M_OPEN.parameters[i].SubParametersLen;
	}
	return true;
}
bool PackageUpdate(UPDATE M_UPDATE, char *UpdateMessage) {
    UpdateMessage[0] = (M_UPDATE.Len >> 8) & 0xFF;
    UpdateMessage[1] = M_UPDATE.Len & 0xFF;
    UpdateMessage[2] = 0x02;
    UpdateMessage[3] = 0x80;
    UpdateMessage[4] = 0xF7;
    UpdateMessage[5] = 0x00;
    UpdateMessage[6] = 0x08;
    char *cpu = (char*)&M_UPDATE.cpu;
    UpdateMessage[7] = cpu[0];
    UpdateMessage[8] = cpu[1];
    UpdateMessage[9] = cpu[2];
    UpdateMessage[10] = cpu[3];
    UpdateMessage[11] = cpu[4];
    UpdateMessage[12] = cpu[5];
    UpdateMessage[13] = cpu[6];
    UpdateMessage[14] = cpu[7];
    UpdateMessage[15] = 0x80;
    UpdateMessage[16] = 0xF8;
    UpdateMessage[17] = 0x00;
    UpdateMessage[18] = 0x08;
    char *memory = (char*)&M_UPDATE.memory;
    UpdateMessage[19] = memory[0];
    UpdateMessage[20] = memory[1];
    UpdateMessage[21] = memory[2];
    UpdateMessage[22] = memory[3];
    UpdateMessage[23] = memory[4];
    UpdateMessage[24] = memory[5];
    UpdateMessage[25] = memory[6];
    UpdateMessage[26] = memory[7];
    return true;
}
bool PackageKeepalive(char* KeepMessage) {
	KeepMessage[0] = 0x00;
	KeepMessage[1] = 0x03;
	KeepMessage[2] = 0x04;
	return true;
}
bool PackageNotification(short ECode, short ESubCode, char* Date, char* NotificationMessage) {
	short DateLen = strlen(Date);
	short Len = DateLen + 5;
	NotificationMessage[0] = (Len >> 8) & 0xFF;
	NotificationMessage[1] = Len & 0xFF;
	NotificationMessage[3] = 0x03;
	NotificationMessage[4] = ECode & 0xFF;
	NotificationMessage[5] = ESubCode & 0xFF;
	for (int i = 0; i < DateLen; i++)
		NotificationMessage[6 + i] = Date[i];
	return true;
}
void OPENOut(OPEN M_OPEN) {
	//屏幕输出
	cout << "MessageLen is:" << M_OPEN.Len << endl;
	cout << "MessageType is: OPEN" << endl;
	cout << "ProtocolVersion is:" << M_OPEN.version << endl;
	cout << "HoldTime is:" << M_OPEN.holdtime << "s" << endl;
	cout << "VREP Identifier is:";
	int i = 0;
	for (i = 0; i < 4; i++) {
		cout << (short) M_OPEN.ID[i];
	}
	cout << endl;
	cout << "Parameters Length is:" << M_OPEN.parametersLen << endl;
	cout << "Parameters Numbers is:" << M_OPEN.ParameterNum << endl;
	for (i = 0; i < M_OPEN.ParameterNum; i++) {
		if (M_OPEN.parameters[i].ParameterType == 1)
			cout << "The number " << i + 1 << " parameter is:Capability Information" << endl;
		else if (M_OPEN.parameters[i].ParameterType == 2)
			cout << "The number " << i + 1 << " parameter is:StreamingZone Name" << endl;
		else if (M_OPEN.parameters[i].ParameterType == 3)
			cout << "The number " << i + 1 << " parameter is:Component Name" << endl;
		cout << "The number " << i + 1 << " parameter's length is:" << M_OPEN.parameters[i].SubParametersLen << endl;
		if (M_OPEN.parameters[i].ParameterType == 1) {
			//	cout<<"The number "<<i+1<<" parameter is:Capability Information"<<endl;
			int position = bytes2int(M_OPEN.parameters[i].ParameterValue);
			cout << "          This Capability's code is:" << M_OPEN.capability[position].M_capabilityCode << endl;
			cout << "          This Capability's length is:" << M_OPEN.capability[position].M_capabilityLen << endl;
			if (M_OPEN.capability[position].M_capabilityCode == 2) {
				int M_SendReceiveCapability = bytes2int(M_OPEN.capability[position].M_capability); //转化SendReceiveCapability值？？？？？？？？？？？？
				cout << "          This Capability's value is:" << M_SendReceiveCapability << endl;
			}
		} else
			cout << "The number " << i + 1 << " parameter's value is:" << M_OPEN.parameters[i].ParameterValue << endl;
	}
}
void UPDATEOut(UPDATE M_UPDATE) {
	//屏幕输出
	cout << "MessageLen is:" << M_UPDATE.Len << endl;
	cout << "MessageType is: UPDATE" << endl;
	cout << "Attributes Numbers is:" << M_UPDATE.AttributeNum << endl;
}
