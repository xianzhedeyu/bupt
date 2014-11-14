#include <arpa/inet.h>
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include "erm_vrep_d6_msg_process.h"
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
			if (M_OPEN.capability[CNum].M_capabilityCode == VREP_ID_D6_OPEN_CapabilityInformation_RouteTypesSupported) //如果属性是RouteTypesSupported，继续拆
			{

				restRTlen = M_OPEN.capability[CNum].M_capabilityLen;
				while (restRTlen > 0) {
					byte *M_AddressFamily;
					byte *M_ApplicationProtocol;
					M_AddressFamily = VREP + begin + 8 + M_OPEN.capability[CNum].M_capabilityLen - restRTlen;
					M_ApplicationProtocol = VREP + begin + 10 + M_OPEN.capability[CNum].M_capabilityLen - restRTlen;
					M_OPEN.routetypessupported[RTNum].AddressFamily = btoi(M_AddressFamily,0); //2字节
					M_OPEN.routetypessupported[RTNum].ApplicationProtocol = btoi(M_ApplicationProtocol,0); //2字节
					RTNum++;
					restRTlen = restRTlen - 4;
				}
				//byte* RT=(byte*)"Route Types Supported";
				toByteArray(RTNum, M_OPEN.capability[CNum].M_capability); //此处值为Route Types Supported的个数
			}
			if (M_OPEN.capability[CNum].M_capabilityCode == VREP_ID_D6_OPEN_CapabilityInformation_SendReceiveCapability) //Send Receive Capability
			{
				byte* SendReceiveCapability;
				SendReceiveCapability = VREP + begin + 8;
				M_OPEN.capability[CNum].M_capability[0] = SendReceiveCapability[0];
				M_OPEN.capability[CNum].M_capability[1] = SendReceiveCapability[1];
				M_OPEN.capability[CNum].M_capability[2] = SendReceiveCapability[2];
				M_OPEN.capability[CNum].M_capability[3] = SendReceiveCapability[3]; //4字节
			}
			//	byte* CI=(byte*)"Capability Information";
			toByteArray(CNum, M_OPEN.parameters[PNum].ParameterValue); //此处值为该Capability所在数组的位置
			CNum++;
		} else {
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
		//TODO:发送错误消息
		cout << "The OPEN Message Length is too short!" << M_OPEN.Len << endl;
	for (i = 0; i < M_OPEN.ParameterNum; i++) {
		if (M_OPEN.parameters[i].ParameterType != 2)
			continue;
		else
			break;
	}
	if (i == M_OPEN.ParameterNum)
		//TODO:缺少必需属性的错误处理
		cout << "There is a lack of Parameter 'StreamingZone' in this OPEN Message" << endl;
	for (i = 0; i < M_OPEN.ParameterNum; i++) {
		if (M_OPEN.parameters[i].ParameterType != 3)
			continue;
		else
			break;
	}
	if (i == M_OPEN.ParameterNum)
		//TODO:缺少必需属性的错误处理
		cout << "There is a lack of Parameter 'Component Name' in this OPEN Message" << endl;
}

void HandlUnkownAttr(Attributes &attr) //未知参数处理
{
}
void ParseRoutes(byte* Attr, short Len, Routes* routes, int &Num) //解析参数WithdrawnRoutes和ReachableRoutes
{
	//	Routes M_Routes[256];
	short restlen = Len;
	short sumlen = 0;
	int RNum = 0; //
	while (restlen > 0) {
		int Rbegin = 6 * RNum + sumlen;
		byte* M_Address;
		routes[RNum].AddFamily = btoi(Attr,Rbegin);
		routes[RNum].AppProtocol = btoi(Attr,Rbegin+2);
		routes[RNum].AddLen = btos(Attr,Rbegin+4);
		M_Address = Attr + Rbegin + 6;
		for (int i = 0; i < routes[RNum].AddLen; i++) {
			routes[RNum].Address[i] = M_Address[i];
			//	cout<<(char)routes[RNum].Address[i];
		}
		sumlen += routes[RNum].AddLen;
		restlen = restlen - (6 + routes[RNum].AddLen);//6＝flag＋type＋length
		RNum++;
	}
	Num = RNum;

}

void ParseNHS(byte* Attr, short Len, Server &M_NextHopServer) //解析参数NextHopServer
{
	M_NextHopServer.AddLen = btos(Attr,4);//开始是有四个保留字节
	int i = 0;
	byte* M_Address = Attr + 6;
	for (i = 0; i < M_NextHopServer.AddLen; i++) {
		M_NextHopServer.Address[i] = M_Address[i];
	}

	M_NextHopServer.ZoneNameLen = btos(Attr,M_NextHopServer.AddLen+6);
	byte* M_ZoneName = Attr + M_NextHopServer.AddLen + 8;
	for (i = 0; i < M_NextHopServer.ZoneNameLen; i++) {
		M_NextHopServer.ZoneName[i] = M_ZoneName[i];
	}

}
void ParseQAMN(byte* Attr, short Len, byte QAMNames[256][256], int &Num) //解析参数QAMNames
{
	short QAMNameLen = 0;
	short sumlen = 0;
	int QNum = 0; //
	while (sumlen < Len) {
		QAMNameLen = btos(Attr,sumlen);
		byte* QAMName = Attr + 2 + sumlen;
		for (int i = 0; i < QAMNameLen; i++) {
			QAMNames[QNum][i] = QAMName[i];
		}
		//cout<<endl;
		sumlen = sumlen + (2 + QAMNameLen);
		QNum++;
	}
	Num = QNum;
}
void ParseEdgeInput(byte* Attr, short Len, Input* EdgeInput, int &Num) //解析参数EdgeInput
{
	short sumlen = 0;
	int INum = 0; //
	while (sumlen < Len) {
		int i = 0;
		short HostLen = 0;
		short GroupNameLen = 0;
		byte M_SubnetMask[256];
		for (i = 0; i < 4; i++) {
			M_SubnetMask[i] = Attr[sumlen + i];
		}
		unsigned int temp = bytes2int2(M_SubnetMask);
		struct in_addr addr1;
		memcpy(&addr1, &temp, 4);
		char *t = inet_ntoa(addr1);
		for (int s = 0; s < strlen(t); s++) {
			EdgeInput[INum].SubnetMask[s] = t[s];
		}

		HostLen = btos(Attr,sumlen+4);
		for (i = 0; i < HostLen; i++) {
			EdgeInput[INum].Host[i] = Attr[sumlen + 6 + i];
		}
		byte* M_PortID = Attr + sumlen + 6 + HostLen;
		byte* M_MaxGroupBW = Attr + sumlen + 10 + HostLen;
		ParsePortID(M_PortID, EdgeInput[INum].portID);
		EdgeInput[INum].MaxGroupBW = bytes2int(M_MaxGroupBW);
		GroupNameLen = btos(Attr,sumlen+14+HostLen);
		for (i = 0; i < GroupNameLen; i++) {
			EdgeInput[INum].GroupName[i] = Attr[sumlen + 16 + HostLen + i];
		}

		sumlen = sumlen + 16 + HostLen + GroupNameLen;
		INum++;
	}
	Num = INum;
}
void ParseQAMP(byte* Attr, short Len, QAMParameters &M_QAMP) //解析参数QAMParameters
{
	M_QAMP.Frequency = bytes2int(Attr);
	short M_Modmode = (short) Attr[4];
	M_QAMP.Modmode[0] = 'Q';
	M_QAMP.Modmode[1] = 'A';
	M_QAMP.Modmode[2] = 'M';
	M_QAMP.Modmode[3] = ' ';
	switch (M_Modmode) {
	case 1:
		M_QAMP.Modmode[4] = '6';
		M_QAMP.Modmode[5] = '4';
		break;
	case 2:
		M_QAMP.Modmode[4] = '2';
		M_QAMP.Modmode[5] = '5';
		M_QAMP.Modmode[6] = '6';
		break;
	case 3:
		M_QAMP.Modmode[4] = '1';
		M_QAMP.Modmode[5] = '0';
		M_QAMP.Modmode[6] = '2';
		M_QAMP.Modmode[7] = '4';
		break;
	}
	M_QAMP.Interleaver = Attr[5];
	M_QAMP.TSID = btos(Attr,6);
	M_QAMP.Annex = Attr[8];
	M_QAMP.Channelwidth = (short) Attr[9];

}
void ParseServiceStatus(byte* Attr, int &ServiceStatus) //解析参数ServiceStatus
{
	byte M_ServiceStatus[4];
	for (int j = 0; j < 4; j++) {
		M_ServiceStatus[j] = Attr[j];
	}
	ServiceStatus = bytes2int(M_ServiceStatus);
}
void ParseUDPMap(byte* Attr, short Len, StaticPorts* SPorts, DynamicPorts* DPorts, int &SNum, int &DNum) //解析参数UDPMap
{
	int SPortNum = 0;
	int DPortNum = 0;
	int i = 0;
	SPortNum = bytes2int(Attr);
	byte* DPTemp = Attr + 4 + 4 * SPortNum;
	byte* SPTemp = NULL;
	DPortNum = bytes2int(DPTemp);
	for (i = 0; i < SPortNum; i++) {
		SPTemp = Attr + 4 + 4 * i;
		SPorts[i].UDPPort = btos(SPTemp,0);
		SPorts[i].ProgramID = btos(SPTemp,2);
	}
	for (i = 0; i < DPortNum; i++) {
		DPTemp = Attr + 8 + 4 * SPortNum + 4 * i;
		DPorts[i].StartingPort = btos(DPTemp,0);
		DPorts[i].StartingPID = btos(DPTemp,2);
		byte* temp = DPTemp + 4;
		DPorts[i].Count = bytes2int(temp);
	}
	SNum = SPortNum;
	DNum = DPortNum;
}
void ParseNHSA(byte* Attr, short Len, NHSAlternates &M_Alternates) //解析参数NextHopServerAlternates
{
	M_Alternates.NumAlternates = btos(Attr,0);
	int ServerLen = 0;
	short sumlen = 2;
	for (int i = 0; i < M_Alternates.NumAlternates; i++) {
		ServerLen = btos(Attr,sumlen);
		byte* M_server = Attr + sumlen + 2;
		for (int j = 0; j < ServerLen; j++) {
			M_Alternates.server[i][j] = M_server[j];
		}
		sumlen = sumlen + 2 + ServerLen;
	}
}
void ParsePortID(byte *Port, PortID &M_Port) {
	M_Port.slotnumber = (short) Port[1];
	M_Port.portnumber = (short) Port[2];
	M_Port.subinterface = (short) Port[3];

}
bool ParseAttr(Attributes* M_attr, int ANum, UPDATE &M_UPDATE) //参数解析函数
{
	for (int i = 0; i < ANum; i++) {
		if (M_attr[i].AttrFlag >> 7)//只使用了这个字节中的最高位
		{
			HandlUnkownAttr(M_attr[i]); //未知参数处理
			continue;
		}
		fprintf(stderr, "type:%d\n", M_attr[i].AttrType);

		switch (M_attr[i].AttrType) {
		case VREP_ID_D6_UPDATE_WithdrawnRoutes:
			ParseRoutes(M_attr[i].AttrValue, M_attr[i].AttrLen, M_UPDATE.WithdrawnRoutes, M_UPDATE.WRoutesNum); //解析参数WithdrawnRoutes
			break;
		case VREP_ID_D6_UPDATE_ReachableRoutes:
			ParseRoutes(M_attr[i].AttrValue, M_attr[i].AttrLen, M_UPDATE.ReachableRoutes, M_UPDATE.RRoutesNum); //解析参数ReachableRoutes
			break;
		case VREP_ID_D6_UPDATE_NextHopServer:
			Server M_NextHopServer;
			ParseNHS(M_attr[i].AttrValue, M_attr[i].AttrLen, M_NextHopServer); //解析参数NextHopServer
			M_UPDATE.NextHopServer = M_NextHopServer;
			break;
		case VREP_ID_D6_UPDATE_QAMNames:
			ParseQAMN(M_attr[i].AttrValue, M_attr[i].AttrLen, M_UPDATE.QAMName, M_UPDATE.QAMNameNum); //解析参数QAMNames
			break;
		case VREP_ID_D6_UPDATE_TotalBandwidth:
			M_UPDATE.totalbw = bytes2int(M_attr[i].AttrValue); //解析参数TotalBandwidth
			break;
		case VREP_ID_D6_UPDATE_AvailableBandwidth:
			M_UPDATE.availablebw = bytes2int(M_attr[i].AttrValue); //解析参数AvailableBandwidth
			break;
		case VREP_ID_D6_UPDATE_Cost:
			M_UPDATE.cost = (short) M_attr[i].AttrValue[0]; //解析参数Cost
			break;
		case VREP_ID_D6_UPDATE_EdgeInput:
			ParseEdgeInput(M_attr[i].AttrValue, M_attr[i].AttrLen, M_UPDATE.EdgeInput, M_UPDATE.EdgeInputNum); //解析参数EdgeInput
			break;
		case VREP_ID_D6_UPDATE_QAMParameters:
			QAMParameters M_QAMP;
			ParseQAMP(M_attr[i].AttrValue, M_attr[i].AttrLen, M_QAMP); //解析参数QAMParameters
			M_UPDATE.QAMP = M_QAMP;
			fprintf(stderr, "********M_UPDATE.QAMP.Annex=%d \n", (short) M_UPDATE.QAMP.Annex);
			break;
		case VREP_ID_D6_UPDATE_UDPMap:
			ParseUDPMap(M_attr[i].AttrValue, M_attr[i].AttrLen, M_UPDATE.SPorts, M_UPDATE.DPorts, M_UPDATE.SPortsNum, M_UPDATE.DPortsNum); //解析参数UDPMap
			break;
		case VREP_ID_D6_UPDATE_ServiceStatus:
			ParseServiceStatus(M_attr[i].AttrValue, M_UPDATE.ServiceStatus); //解析参数ServiceStatus
			break;
		case VREP_ID_D6_UPDATE_MaxMpegFlows:
			M_UPDATE.MaxMpegFlows = bytes2int(M_attr[i].AttrValue); //解析参数MaxMpegFlows
			break;
		case VREP_ID_D6_UPDATE_NextHopServerAlternates:
			NHSAlternates M_Alternates;
			ParseNHSA(M_attr[i].AttrValue, M_attr[i].AttrLen, M_Alternates); //解析参数NextHopServerAlternates
			M_UPDATE.Alternates = M_Alternates;
			break;
		case VREP_ID_D6_UPDATE_OutputPort:
			ParsePortID(M_attr[i].AttrValue, M_UPDATE.OutputPort); //解析参数OutputPort
			break;
		default:
			//TODO:未知代码参数处理
			return false;
			break;
		}
	}
	return true;
}
bool ParseUPDATE(byte VREP[], UPDATE &M_UPDATE) //解析UPDATE消息函数
{
	short M_Len = 0;
	int ANum = 0;
	M_Len = btos(VREP,0);
	short restLen = M_Len - 3; //解析消息的剩余长度,初始为去掉头的长度
	short len = 0; //取完的参数长度之和
	while (restLen > 0) {
		int begin = 3 + 4 * ANum + len; //起点指针
		M_UPDATE.attributes[ANum].AttrFlag = VREP[begin];

		M_UPDATE.attributes[ANum].AttrType = VREP[begin + 1];
		M_UPDATE.attributes[ANum].AttrLen = btos(VREP,begin+2);
		M_UPDATE.attributes[ANum].AttrValue = VREP + begin + 4;
		printf("flag: %x\n", VREP[begin]);
		printf("type: %x\n", VREP[begin + 1]);
		printf("length: %x\n", M_UPDATE.attributes[ANum].AttrLen);
		printf("val: %x\n",VREP + begin + 4);
		cout << "reslen: " << restLen << endl;
		len += M_UPDATE.attributes[ANum].AttrLen;
		restLen = restLen - (4 + M_UPDATE.attributes[ANum].AttrLen);
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
void InspectUPDATE(UPDATE &M_UPDATE) {
	//判断强制参数及次序等格式
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
	//OpenMessage[4]=0x00;//保留
	OpenMessage[5] = (M_OPEN.holdtime >> 8) & 0xFF;//holdtime高字节
	OpenMessage[6] = M_OPEN.holdtime & 0xFF;//holdtime低字节
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
		for (int j = 0; j < M_OPEN.parameters[i].SubParametersLen; j++)
			OpenMessage[21 + sumlen + j] = (char) M_OPEN.parameters[i].ParameterValue[j]; //参数值
		sumlen = sumlen + 4 + M_OPEN.parameters[i].SubParametersLen;
	}
	OpenMessage[4] = 0x00;
	OpenMessage[7] = 0x00;//added by orion
	OpenMessage[8] = 0x00;
	OpenMessage[9] = 0x00;
	OpenMessage[10] = 0x00;
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
		else if (M_OPEN.parameters[i].ParameterType == 4)
			cout << "The number " << i + 1 << " parameter is:Vendor Specific String" << endl;
		cout << "The number " << i + 1 << " parameter's length is:" << M_OPEN.parameters[i].SubParametersLen << endl;
		if (M_OPEN.parameters[i].ParameterType == 1) {
			//	cout<<"The number "<<i+1<<" parameter is:Capability Information"<<endl;
			int position = bytes2int(M_OPEN.parameters[i].ParameterValue);
			cout << "          This Capability's code is:" << M_OPEN.capability[position].M_capabilityCode << endl;
			cout << "          This Capability's length is:" << M_OPEN.capability[position].M_capabilityLen << endl;
			if (M_OPEN.capability[position].M_capabilityCode == 1) {
				cout << "          This Capability is Route Types Supported" << endl;
				int RTnums = bytes2int(M_OPEN.capability[position].M_capability);
				for (int j = 0; j < RTnums; j++) {
					cout << "          The number " << j + 1 << " Supported Route Type's AddressFamily is:" << M_OPEN.routetypessupported[j].AddressFamily << endl;
					cout << "          The number " << j + 1 << " Supported Route Type's ApplicationProtocol is:" << M_OPEN.routetypessupported[j].ApplicationProtocol << endl;
				}
			} else {
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
	for (int i = 0; i < M_UPDATE.AttributeNum; i++) {
		if (M_UPDATE.attributes[i].AttrFlag >> 7) {
			cout << "Unknow Attributes" << endl;
			continue;
		}
		int k = 0;
		int j = 0;
		switch (M_UPDATE.attributes[i].AttrType) {
		case VREP_ID_D6_UPDATE_WithdrawnRoutes:
			cout << "WithdrawnRoutes" << endl;
			for (k = 0; k < M_UPDATE.WRoutesNum; k++) {
				cout << "The number " << k + 1 << " Routes's AddFamily is:" << M_UPDATE.WithdrawnRoutes[k].AddFamily << endl;
				cout << "The number " << k + 1 << " Routes's AppProtocol is:" << M_UPDATE.WithdrawnRoutes[k].AppProtocol << endl;
				cout << "The number " << k + 1 << " Routes's AddLen is:" << M_UPDATE.WithdrawnRoutes[k].AddLen << endl;
				cout << "The number " << k + 1 << " Routes's Address is:";
				for (j = 0; j < M_UPDATE.WithdrawnRoutes[k].AddLen; j++) {
					cout << (char) M_UPDATE.WithdrawnRoutes[k].Address[j];
				}
				cout << endl;
			}
			break;
		case VREP_ID_D6_UPDATE_ReachableRoutes:
			cout << "ReachableRoutes" << endl;
			for (k = 0; k < M_UPDATE.RRoutesNum; k++) {
				cout << "The number " << k + 1 << " Routes's AddFamily is:" << M_UPDATE.ReachableRoutes[k].AddFamily << endl;
				cout << "The number " << k + 1 << " Routes's AppProtocol is:" << M_UPDATE.ReachableRoutes[k].AppProtocol << endl;
				cout << "The number " << k + 1 << " Routes's AddLen is:" << M_UPDATE.ReachableRoutes[k].AddLen << endl;
				cout << "The number " << k + 1 << " Routes's Address is:";
				for (j = 0; j < M_UPDATE.ReachableRoutes[k].AddLen; j++) {
					cout << (char) M_UPDATE.ReachableRoutes[k].Address[j];
				}
				cout << endl;
			}
			break;
		case VREP_ID_D6_UPDATE_NextHopServer:
			cout << "NextHopServer" << endl;
			cout << "AddLen is:" << M_UPDATE.NextHopServer.AddLen << endl;
			cout << "Address is:";
			for (k = 0; k < M_UPDATE.NextHopServer.AddLen; k++) {
				cout << (char) M_UPDATE.NextHopServer.Address[k];
			}
			cout << endl;
			cout << "ZoneNameLen is:" << M_UPDATE.NextHopServer.ZoneNameLen << endl;
			cout << "ZoneName is:";
			for (k = 0; k < M_UPDATE.NextHopServer.ZoneNameLen; k++) {
				cout << (char) M_UPDATE.NextHopServer.ZoneName[k];
			}
			cout << endl;
			break;
		case VREP_ID_D6_UPDATE_QAMNames:
			cout << "QAMNames" << endl;
			for (k = 0; k < M_UPDATE.QAMNameNum; k++) {
				cout << "The number " << k + 1 << " QAMName is:";
				for (j = 0; j < 256; j++) {
					cout << M_UPDATE.QAMName[k][j];
				}
				cout << endl;
			}
			break;
		case VREP_ID_D6_UPDATE_TotalBandwidth:
			cout << "TotalBandwidth: ";
			cout << M_UPDATE.totalbw << endl;
			break;
		case VREP_ID_D6_UPDATE_AvailableBandwidth:
			cout << "AvailableBandwidth: ";
			cout << M_UPDATE.availablebw << endl;
			break;
		case VREP_ID_D6_UPDATE_Cost:
			cout << "Cost: ";
			cout << M_UPDATE.cost << endl;
			break;
		case VREP_ID_D6_UPDATE_EdgeInput:
			cout << "EdgeInput" << endl;
			for (k = 0; k < M_UPDATE.EdgeInputNum; k++) {
				cout << "The number " << k + 1 << " EdgeInput's SubnetMask is:";
				//	for(j=0;j<strlen(M_UPDATE.EdgeInput[i].SubnetMask);j++)
				//	{
				cout << M_UPDATE.EdgeInput[i].SubnetMask;
				//	if(j<3)
				//		cout<<'.';
				//	}
				//	cout<<bytes2int(M_UPDATE.EdgeInput[i].SubnetMask);
				cout << endl;
				cout << "The number " << k + 1 << " EdgeInput's Host is:";
				for (j = 0; j < 256; j++) {
					cout << M_UPDATE.EdgeInput[k].Host[j];
				}
				cout << endl;
				cout << "The number " << k + 1 << " EdgeInput's Port is:" << M_UPDATE.EdgeInput[k].portID.portnumber << endl;
				cout << "The number " << k + 1 << " EdgeInput's MaxGroupBW is:" << M_UPDATE.EdgeInput[k].MaxGroupBW << endl;
				cout << "The number " << k + 1 << " EdgeInput's GroupName is:";
				for (j = 0; j < 256; j++) {
					cout << M_UPDATE.EdgeInput[k].GroupName[j];
				}
				cout << endl;
			}
			break;
		case VREP_ID_D6_UPDATE_QAMParameters:
			cout << "QAMParameters" << endl;
			cout << "QAM Frequency is:" << M_UPDATE.QAMP.Frequency << endl;
			cout << "QAM Modmode is:" << M_UPDATE.QAMP.Modmode << endl;
			cout << "QAM Interleaver is:" << (short) M_UPDATE.QAMP.Interleaver << endl;
			cout << "QAM TSID is:" << M_UPDATE.QAMP.TSID << endl;
			cout << "QAM Annex is:" << (short) M_UPDATE.QAMP.Annex << endl;
			cout << "QAM Channelwidth is:" << M_UPDATE.QAMP.Channelwidth << endl;
			break;
		case VREP_ID_D6_UPDATE_UDPMap:
			cout << "UDPMap" << endl;
			cout << "There is " << M_UPDATE.SPortsNum << " StaticPorts" << endl;
			cout << "There is " << M_UPDATE.DPortsNum << " DynamicPorts" << endl;
			for (j = 0; j < M_UPDATE.SPortsNum; j++) {
				cout << "The number " << j + 1 << " StaticPorts's UDPPort is:" << M_UPDATE.SPorts[j].UDPPort << endl;
				cout << "The number " << j + 1 << " StaticPorts's ProgramID is:" << M_UPDATE.SPorts[j].ProgramID << endl;
			}
			for (j = 0; j < M_UPDATE.DPortsNum; j++) {
				cout << "The number " << j + 1 << " DynamicPorts's StartingPort is:" << M_UPDATE.DPorts[j].StartingPort << endl;
				cout << "The number " << j + 1 << " DynamicPorts's StartingPID is:" << M_UPDATE.DPorts[j].StartingPID << endl;
				cout << "The number " << j + 1 << " DynamicPorts's Count is:" << M_UPDATE.DPorts[j].Count << endl;
			}
			break;
		case VREP_ID_D6_UPDATE_ServiceStatus:
			cout << "ServiceStatus:";
			cout << M_UPDATE.ServiceStatus << endl;
			break;
		case VREP_ID_D6_UPDATE_MaxMpegFlows:
			cout << "MaxMpegFlows:";
			cout << M_UPDATE.MaxMpegFlows << endl;
			break;
		case VREP_ID_D6_UPDATE_NextHopServerAlternates:
			cout << "NextHopServerAlternates" << endl;
			break;
		case VREP_ID_D6_UPDATE_OutputPort:
			cout << "OutputPort" << endl;
			cout << "slot number is:" << M_UPDATE.OutputPort.slotnumber << endl;
			cout << "port number is:" << M_UPDATE.OutputPort.portnumber << endl;
			cout << "sub interface is:" << M_UPDATE.OutputPort.subinterface << endl;
			break;
		default:
			//TODO:未知代码参数处理
			break;
		}
	}
}
