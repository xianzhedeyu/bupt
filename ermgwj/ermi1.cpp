#include <arpa/inet.h>
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include "ermi1.h"
#include "ermi_def.h"

static int ermibytes2int(byte* b) //字节数组(4位)转化为int型,整型的低字节位是字节数组的高位
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
static unsigned int ermibytes2int2(byte* b) //字节数组(4位)转化为int型,整型的高字节位是字节数组的高位
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
//!!!!
bool parse_drrp_open(byte* DRRP, OPEN &MSG_OPEN) //OPEN消息解析函数
{
	short MSG_Len = 0; //消息长度
	short MSG_version = 0; //协议版本
	short MSG_holdtime = 0; //HoldTime
	short MSG_parametersLen = 0; //参数


	int i = 0; //计数用

	MSG_Len = btos(DRRP,0);
	fprintf(stderr, "MSG_len:%d\n", MSG_Len);
	MSG_version = (short) DRRP[3];
	fprintf(stderr, "MSG_version:%d\n", MSG_version);
	MSG_holdtime = btos(DRRP,5);
	fprintf(stderr, "MSG_holdtime:%d\n", MSG_holdtime);
	//...
//需要添加AddressDomain
	byte* AD = DRRP + 7;
	for (i = 0; i < 4; i++)
		MSG_OPEN.AddressDomain[i] = AD[i];
	
	byte* ID = DRRP + 11;
	for (i = 0; i < 4; i++)
		MSG_OPEN.ID[i] = ID[i];
	MSG_parametersLen = btos(DRRP,15);
	fprintf(stderr, "MSG_parametersLen:%d\n", MSG_parametersLen);

	short restPlen = MSG_parametersLen; //用于计算剩余参数长度
	short restRTlen = 0; //用于计算Capability为RouteTypes时剩余capabilityLen
	int PNum = 0; // 参数个数
	int CNum = 0; //Capability个数
	int RTNum = 0; //RouteTypesSupported个数
	short len = 0; //取完的参数长度之和
	while (restPlen > 0) {
		int begin = 17 + 4 * PNum + len; //起点指针
		MSG_OPEN.parameters[PNum].ParameterType = btos(DRRP,begin);
		fprintf(stderr, "ParameterType:%d\n", MSG_OPEN.parameters[PNum].ParameterType);
		MSG_OPEN.parameters[PNum].SubParametersLen = btos(DRRP,begin+2);
		fprintf(stderr, "SubParametersLen:%d\n", MSG_OPEN.parameters[PNum].SubParametersLen);
		if (MSG_OPEN.parameters[PNum].ParameterType == DRRP_ID_ERMI1_OPEN_CapabilityInformation) //若为Capability Information参数
		{
			MSG_OPEN.capability[CNum].MSG_capabilityCode = btos(DRRP,begin+4);
			fprintf(stderr, "MSG_capabilityCode:%d\n", MSG_OPEN.capability[CNum].MSG_capabilityCode);
			MSG_OPEN.capability[CNum].MSG_capabilityLen = btos(DRRP,begin+6);
			fprintf(stderr, "MSG_capabilityLen:%d\n", MSG_OPEN.capability[CNum].MSG_capabilityLen);
			if (MSG_OPEN.capability[CNum].MSG_capabilityCode == DRRP_ID_ERMI1_OPEN_CapabilityInformation_RouteTypesSupported) //如果属性是RouteTypesSupported，继续拆
			{

				restRTlen = MSG_OPEN.capability[CNum].MSG_capabilityLen;
				while (restRTlen > 0) {
					byte *MSG_AddressFamily;
					byte *MSG_ApplicationProtocol;
					MSG_AddressFamily = DRRP + begin + 8 + MSG_OPEN.capability[CNum].MSG_capabilityLen - restRTlen;
					MSG_ApplicationProtocol = DRRP + begin + 10 + MSG_OPEN.capability[CNum].MSG_capabilityLen - restRTlen;
					MSG_OPEN.routetypessupported[RTNum].AddressFamily = btoi(MSG_AddressFamily,0); //2字节
					MSG_OPEN.routetypessupported[RTNum].ApplicationProtocol = btoi(MSG_ApplicationProtocol,0); //2字节
					RTNum++;
					restRTlen = restRTlen - 4;
				}
				//byte* RT=(byte*)"Route Types Supported";
				toByteArray(RTNum, MSG_OPEN.capability[CNum].MSG_capability); //此处值为Route Types Supported的个数
			}
			if (MSG_OPEN.capability[CNum].MSG_capabilityCode == DRRP_ID_ERMI1_OPEN_CapabilityInformation_SendReceiveCapability) //Send Receive Capability
			{
				byte* SendReceiveCapability;
				SendReceiveCapability = DRRP + begin + 8;
				MSG_OPEN.capability[CNum].MSG_capability[0] = SendReceiveCapability[0];
				MSG_OPEN.capability[CNum].MSG_capability[1] = SendReceiveCapability[1];
				MSG_OPEN.capability[CNum].MSG_capability[2] = SendReceiveCapability[2];
				MSG_OPEN.capability[CNum].MSG_capability[3] = SendReceiveCapability[3]; //4字节
			}

			//	byte* CI=(byte*)"Capability Information";
			toByteArray(CNum, MSG_OPEN.parameters[PNum].ParameterValue); //此处值为该Capability所在数组的位置
			CNum++;
		} else {
			byte* ParameterValue = DRRP + begin + 4;
			//	byte MSG_ParameterValue[256];
			for (i = 0; i < MSG_OPEN.parameters[PNum].SubParametersLen; i++) {
				MSG_OPEN.parameters[PNum].ParameterValue[i] = ParameterValue[i];
				fprintf(stderr, "ParameterValue:%d\n", MSG_OPEN.parameters[PNum].ParameterValue[i]);
			}
			//	MSG_OPEN.parameters[PNum].ParameterValue=MSG_ParameterValue;
		}
		len += MSG_OPEN.parameters[PNum].SubParametersLen;
		restPlen = restPlen - (4 + MSG_OPEN.parameters[PNum].SubParametersLen);
		PNum++;
	}

	//写入结构
	MSG_OPEN.Len = MSG_Len;
	MSG_OPEN.type = '1';
	MSG_OPEN.version = MSG_version;
	MSG_OPEN.holdtime = MSG_holdtime;
	MSG_OPEN.parametersLen = MSG_parametersLen;
	MSG_OPEN.ParameterNum = PNum;

	return true;
}
void ins_drrp_open(OPEN &MSG_OPEN) //OPEN消息检查
{
	int i = 0;
	if (MSG_OPEN.Len < 17)
		//TODO:发送错误消息
		cout << "The OPEN Message Length is too short!" << MSG_OPEN.Len << endl;
	for (i = 0; i < MSG_OPEN.ParameterNum; i++) {
		if (MSG_OPEN.parameters[i].ParameterType != 2)
			continue;
		else
			break;
	}
	if (i == MSG_OPEN.ParameterNum)
		//TODO:缺少必需属性的错误处理
		cout << "There is a lack of Parameter 'StreamingZone' in this OPEN Message" << endl;
	for (i = 0; i < MSG_OPEN.ParameterNum; i++) {
		if (MSG_OPEN.parameters[i].ParameterType != 3)
			continue;
		else
			break;
	}
	if (i == MSG_OPEN.ParameterNum)
		//TODO:缺少必需属性的错误处理
		cout << "There is a lack of Parameter 'Component Name' in this OPEN Message" << endl;
}

void hdl_undrrp_attr(Attributes &attr) //未知参数处理
{
}
void parse_drrp_route(byte* Attr, short Len, Routes* routes, int &Num) //解析参数WithdrawnRoutes和ReachableRoutes
{
	//	Routes MSG_Routes[256];
	short restlen = Len;
	short sumlen = 0;
	int RNum = 0; //
	while (restlen > 0) {
		int Rbegin = 6 * RNum + sumlen;
		byte* MSG_Address;
		routes[RNum].AddFamily = btoi(Attr,Rbegin);
		routes[RNum].AppProtocol = btoi(Attr,Rbegin+2);
		routes[RNum].AddLen = btos(Attr,Rbegin+4);
		MSG_Address = Attr + Rbegin + 6;
		for (int i = 0; i < routes[RNum].AddLen; i++) {
			routes[RNum].Address[i] = MSG_Address[i];
			//	cout<<(char)routes[RNum].Address[i];
		}
		sumlen += routes[RNum].AddLen;
		restlen = restlen - (6 + routes[RNum].AddLen);//6＝flag＋type＋length
		RNum++;
	}
	Num = RNum;

}

void parse_drrp_nhs(byte* Attr, short Len, Server &MSG_NextHopServer) //解析参数NextHopServer
{
	MSG_NextHopServer.NextHopAddressDomain = ermibytes2int(Attr);//开始是有四个保留字节
	MSG_NextHopServer.Length = ermibytes2int(Attr + 4);//开始是有四个保留字节
	int i = 0;
	byte* MSG_Address = Attr + 6;
	for (i = 0; i < MSG_NextHopServer.Length; i++) {
		MSG_NextHopServer.Server[i] = MSG_Address[i];
	}
}

bool parse_drrp_attr(Attributes* MSG_attr, int ANum, UPDATE &MSG_UPDATE) //参数解析函数
{
	for (int i = 0; i < ANum; i++) {
		if (MSG_attr[i].AttrFlag >> 7)//只使用了这个字节中的最高位
		{
			hdl_undrrp_attr(MSG_attr[i]); //未知参数处理
			continue;
		}
		fprintf(stderr, "type:%d\n", MSG_attr[i].AttrType);

		int j = 0;
		switch (MSG_attr[i].AttrType) {
		case DRRP_ID_ERMI1_UPDATE_WithdrawnRoutes:
			parse_drrp_route(MSG_attr[i].AttrValue, MSG_attr[i].AttrLen, MSG_UPDATE.WithdrawnRoutes, MSG_UPDATE.WRoutesNum); //解析参数WithdrawnRoutes
			break;
		case DRRP_ID_ERMI1_UPDATE_ReachableRoutes:
			parse_drrp_route(MSG_attr[i].AttrValue, MSG_attr[i].AttrLen, MSG_UPDATE.ReachableRoutes, MSG_UPDATE.RRoutesNum); //解析参数ReachableRoutes
			break;
		case DRRP_ID_ERMI1_UPDATE_NextHopServer:
			Server MSG_NextHopServer;
			parse_drrp_nhs(MSG_attr[i].AttrValue, MSG_attr[i].AttrLen, MSG_NextHopServer); //解析参数NextHopServer
			MSG_UPDATE.NextHopServer = MSG_NextHopServer;
			break;
		case DRRP_ID_ERMI1_UPDATE_TotalBandwidth:
			MSG_UPDATE.totalbw = ermibytes2int(MSG_attr[i].AttrValue); //解析参数TotalBandwidth
			break;
		
		case DRRP_ID_ERMI1_UPDATE_ServiceStatus:
			MSG_UPDATE.ServiceStatus = ermibytes2int(MSG_attr[i].AttrValue);
			break;		
		default:
			//TODO:未知代码参数处理
			return false;
			//break;
		}
	}
	return true;
}
bool parse_drrp_update(byte DRRP[], UPDATE &MSG_UPDATE) //解析UPDATE消息函数
{
	short MSG_Len = 0;
	int ANum = 0;
	MSG_Len = btos(DRRP,0);
	short restLen = MSG_Len - 3; //解析消息的剩余长度,初始为去掉头的长度
	short len = 0; //取完的参数长度之和
	while (restLen > 0) {
		int begin = 3 + 4 * ANum + len; //起点指针
		MSG_UPDATE.attributes[ANum].AttrFlag = DRRP[begin];
		MSG_UPDATE.attributes[ANum].AttrType = DRRP[begin + 1];
		MSG_UPDATE.attributes[ANum].AttrLen = btos(DRRP,begin+2);
		MSG_UPDATE.attributes[ANum].AttrValue = DRRP + begin + 4;
		len += MSG_UPDATE.attributes[ANum].AttrLen;
		restLen = restLen - (4 + MSG_UPDATE.attributes[ANum].AttrLen);
		ANum++;
	}

	//写入结构
	MSG_UPDATE.Len = MSG_Len;
	MSG_UPDATE.type = '2';
	MSG_UPDATE.AttributeNum = ANum;

	if (parse_drrp_attr(MSG_UPDATE.attributes, ANum, MSG_UPDATE))
		return true;
	else
		return false;
}
void ins_drrp_update(UPDATE &MSG_UPDATE) {
	//判断强制参数及次序等格式
}

bool parse_drrp_notification(byte DRRP[]) {
	short MSG_Len = btos(DRRP,0);
	short MSG_Code = (short) DRRP[3];
	short MSG_Subcode = (short) DRRP[4];
	short dateLen = MSG_Len - 5;
	byte MSG_Date[256];
	int i = 0;
	for (i = 0; i < dateLen; i++)
		MSG_Date[i] = DRRP[5 + i];
	switch (MSG_Code) {
	case DRRP_ID_ERMI1_NOTIFICATION_MessageHeaderError: //Message Header Error
		if (MSG_Subcode == DRRP_ID_ERMI1_NOTIFICATION_MessageHeaderError_BadMessageLength) //Bad Message Length
			//TODO:错误处理
			cout << "Bad Message Length!" << endl;
		if (MSG_Subcode == DRRP_ID_ERMI1_NOTIFICATION_MessageHeaderError_BadMessageType) //Bad Message Type
			//TODO:错误处理
			cout << "Bad Message Type!" << endl;
		break;
	case DRRP_ID_ERMI1_NOTIFICATION_OPENMessageError: //OPEN Message Error
		if (MSG_Subcode == DRRP_ID_ERMI1_NOTIFICATION_OPENMessageError_UnsupportedVersionNumber) //Unsupported Version Number
			//TODO:错误处理
			cout << "Unsupported Version Number!" << endl;
		if (MSG_Subcode == DRRP_ID_ERMI1_NOTIFICATION_OPENMessageError_BadPeerAddressDomain) //Bad Peer Address Domain
			//TODO:错误处理
			cout << "Bad Peer Address Domain!" << endl;
		if (MSG_Subcode == DRRP_ID_ERMI1_NOTIFICATION_OPENMessageError_BadDRRPIdentifier) //Bad DRRP Identifier
			//TODO:错误处理
			cout << "Bad DRRP Identifier!" << endl;
		if (MSG_Subcode == DRRP_ID_ERMI1_NOTIFICATION_OPENMessageError_UnsupportedOptionalParameter) //Unsupported Optional Parameter
			//TODO:错误处理
			cout << "Unsupported Optional Parameter!" << endl;
		if (MSG_Subcode == DRRP_ID_ERMI1_NOTIFICATION_OPENMessageError_UnacceptableHoldTime) //Unacceptable Hold Time
			//TODO:错误处理
			cout << "Unacceptable Hold Time!" << endl;
		if (MSG_Subcode == DRRP_ID_ERMI1_NOTIFICATION_OPENMessageError_UnsupportedCapability) //Unsupported Capability
			//TODO:错误处理
			cout << "Unsupported Capability!" << endl;
		if (MSG_Subcode == DRRP_ID_ERMI1_NOTIFICATION_OPENMessageError_CapabilityMismatch) //Capability Mismatch
			//TODO:错误处理
			cout << "Capability Mismatch!" << endl;
		break;
	case DRRP_ID_ERMI1_NOTIFICATION_UPDATEMessageError: //UPDATE Message Error
		if (MSG_Subcode == DRRP_ID_ERMI1_NOTIFICATION_UPDATEMessageError_MalformedAttributeList) //Malformed Attribute List
			//TODO:错误处理
			cout << "Malformed Attribute List!" << endl;
		if (MSG_Subcode == DRRP_ID_ERMI1_NOTIFICATION_UPDATEMessageError_nrecognizedWellknownAttribute) //Unrecognized Well-known Attribute
			//TODO:错误处理
			cout << "Unrecognized Well-known Attribute!" << endl;
		if (MSG_Subcode == DRRP_ID_ERMI1_NOTIFICATION_UPDATEMessageError_MissingWellknownMandatoryAttribute) //Missing Well-known Mandatory Attribute
			//TODO:错误处理
			cout << "Missing Well-known Mandatory Attribute!" << endl;
		if (MSG_Subcode == DRRP_ID_ERMI1_NOTIFICATION_UPDATEMessageError_AttributeFlagsError) //Attribute Flags Error
			//TODO:错误处理
			cout << "Attribute Flags Error!" << endl;
		if (MSG_Subcode == DRRP_ID_ERMI1_NOTIFICATION_UPDATEMessageError_AttributeLengthError) //Attribute Length Error
			//TODO:错误处理
			cout << "Attribute Length Error!" << endl;
		if (MSG_Subcode == DRRP_ID_ERMI1_NOTIFICATION_UPDATEMessageError_InvalidAttribute) //Invalid Attribute
			//TODO:错误处理
			cout << "Invalid Attribute!" << endl;
		break;
	case DRRP_ID_ERMI1_NOTIFICATION_HoldTimerExpired: //Hold Timer Expired
		//TODO:错误处理
		cout << "Hold Timer Expired!" << endl;
		break;
	case DRRP_ID_ERMI1_NOTIFICATION_FiniteStateMachineError: //Finite State Machine Error
		//TODO:错误处理
		cout << "Finite State Machine Error!" << endl;
		break;
	case DRRP_ID_ERMI1_NOTIFICATION_Cease: //Cease
		//TODO:错误处理
		cout << "Cease!" << endl;
		for (i = 0; i < dateLen; i++)
			cout << MSG_Date[i]; //上面应该也输出错误原因，暂不写
		break;
	default:
		break;

	}
	return true;
}
bool drrp_parse(byte DRRP[]) {
	switch (DRRP[2]) {
	case DRRP_ID_ERMI1_OPEN:
		OPEN MSG_OPEN;
		if (parse_drrp_open(DRRP, MSG_OPEN)) {
			print_drrp_open(MSG_OPEN);
			ins_drrp_open(MSG_OPEN);
		}
		//将MSG_OPEN写入数据库
		break;
	case DRRP_ID_ERMI1_UPDATE:
		UPDATE MSG_UPDATE;
		if (parse_drrp_update(DRRP, MSG_UPDATE)) {
			print_drrp_update(MSG_UPDATE);
			ins_drrp_update(MSG_UPDATE);
		}
		break;
	case DRRP_ID_ERMI1_NOTIFICATION:
		cout << "MessageType is: NOTIFICATION" << endl;
		parse_drrp_notification(DRRP);
		//上报错误
		break;
	case DRRP_ID_ERMI1_KEEPALIVE:
		//发送KEEPALIVE消息
		char keepMsg[3];
		pack_drrp_keepalive(keepMsg);
		cout << "MessageType is: KEEPALIVE" << endl;
		break;
	default:
		//TODO:错误处理
		cout << "MessageType ERROR!" << endl;
		cout << (short) DRRP[2] << endl;
		return false;		
	}

	return true;
}

bool pack_drrp_open(OPEN MSG_OPEN, char* OpenMessage) {
	OpenMessage[0] = (MSG_OPEN.Len >> 8) & 0xFF;//长度高字节
	OpenMessage[1] = MSG_OPEN.Len & 0xFF;//长度低字节
	OpenMessage[2] = 0x01;//OPEN类型
	OpenMessage[3] = 0x02;//版本
	//OpenMessage[4]=0x00;//保留
	OpenMessage[5] = (MSG_OPEN.holdtime >> 8) & 0xFF;//holdtime高字节
	OpenMessage[6] = MSG_OPEN.holdtime & 0xFF;//holdtime低字节
	OpenMessage[11] = (char) MSG_OPEN.ID[0];//DRRP ID
	OpenMessage[12] = (char) MSG_OPEN.ID[1];
	OpenMessage[13] = (char) MSG_OPEN.ID[2];
	OpenMessage[14] = (char) MSG_OPEN.ID[3];
	OpenMessage[15] = (MSG_OPEN.parametersLen >> 8) & 0xFF;//参数长度高字节
	OpenMessage[16] = MSG_OPEN.parametersLen & 0xFF;//参数长度低字节
	int sumlen = 0;
	for (int i = 0; i < MSG_OPEN.ParameterNum; i++) {
		OpenMessage[17 + sumlen] = (MSG_OPEN.parameters[i].ParameterType >> 8) & 0xFF;//参数类型高字节
		OpenMessage[18 + sumlen] = MSG_OPEN.parameters[i].ParameterType & 0xFF;//参数类型低字节
		OpenMessage[19 + sumlen] = (MSG_OPEN.parameters[i].SubParametersLen >> 8) & 0xFF;//参数长度高字节
		OpenMessage[20 + sumlen] = MSG_OPEN.parameters[i].SubParametersLen & 0xFF;//参数长度低字节
		for (int j = 0; j < MSG_OPEN.parameters[i].SubParametersLen; j++)
			OpenMessage[21 + sumlen + j] = (char) MSG_OPEN.parameters[i].ParameterValue[j]; //参数值
		sumlen = sumlen + 4 + MSG_OPEN.parameters[i].SubParametersLen;
	}
	OpenMessage[4] = 0x00;
	OpenMessage[7] = 0x00;//added by orion
	OpenMessage[8] = 0x00;
	OpenMessage[9] = 0x00;
	OpenMessage[10] = 0x00;
	return true;
}
bool pack_drrp_keepalive(char* KeepMessage) {
	KeepMessage[0] = 0x00;
	KeepMessage[1] = 0x03;
	KeepMessage[2] = 0x04;
	return true;
}
bool pack_drrp_notification(short ECode, short ESubCode, char* Date, char* NotificationMessage) {
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
void print_drrp_open(OPEN MSG_OPEN) {
	//屏幕输出
	cout << "MessageLen is:" << MSG_OPEN.Len << endl;
	cout << "MessageType is: OPEN" << endl;
	cout << "ProtocolVersion is:" << MSG_OPEN.version << endl;
	cout << "HoldTime is:" << MSG_OPEN.holdtime << "s" << endl;
	cout << "DRRP Identifier is:";
	int i = 0;
	for (i = 0; i < 4; i++) {
		cout << (short) MSG_OPEN.ID[i];
	}
	cout << endl;
	cout << "Parameters Length is:" << MSG_OPEN.parametersLen << endl;
	cout << "Parameters Numbers is:" << MSG_OPEN.ParameterNum << endl;
	for (i = 0; i < MSG_OPEN.ParameterNum; i++) {
		if (MSG_OPEN.parameters[i].ParameterType == 1)
			cout << "The number " << i + 1 << " parameter is:Capability Information" << endl;
		else if (MSG_OPEN.parameters[i].ParameterType == 2)
			cout << "The number " << i + 1 << " parameter is:StreamingZone Name" << endl;
		else if (MSG_OPEN.parameters[i].ParameterType == 3)
			cout << "The number " << i + 1 << " parameter is:Component Name" << endl;
		else if (MSG_OPEN.parameters[i].ParameterType == 4)
			cout << "The number " << i + 1 << " parameter is:Vendor Specific String" << endl;
		cout << "The number " << i + 1 << " parameter's length is:" << MSG_OPEN.parameters[i].SubParametersLen << endl;
		if (MSG_OPEN.parameters[i].ParameterType == 1) {
			//	cout<<"The number "<<i+1<<" parameter is:Capability Information"<<endl;
			int position = ermibytes2int(MSG_OPEN.parameters[i].ParameterValue);
			cout << "          This Capability's code is:" << MSG_OPEN.capability[position].MSG_capabilityCode << endl;
			cout << "          This Capability's length is:" << MSG_OPEN.capability[position].MSG_capabilityLen << endl;
			if (MSG_OPEN.capability[position].MSG_capabilityCode == 1) {
				cout << "          This Capability is Route Types Supported" << endl;
				int RTnums = ermibytes2int(MSG_OPEN.capability[position].MSG_capability);
				for (int j = 0; j < RTnums; j++) {
					cout << "          The number " << j + 1 << " Supported Route Type's AddressFamily is:" << MSG_OPEN.routetypessupported[j].AddressFamily << endl;
					cout << "          The number " << j + 1 << " Supported Route Type's ApplicationProtocol is:" << MSG_OPEN.routetypessupported[j].ApplicationProtocol << endl;
				}
			} else {
				int MSG_SendReceiveCapability = ermibytes2int(MSG_OPEN.capability[position].MSG_capability); //转化SendReceiveCapability值？？？？？？？？？？？？
				cout << "          This Capability's value is:" << MSG_SendReceiveCapability << endl;
			}
		} else
			cout << "The number " << i + 1 << " parameter's value is:" << MSG_OPEN.parameters[i].ParameterValue << endl;
	}
}
void print_drrp_update(UPDATE MSG_UPDATE) {
	//屏幕输出
	cout << "MessageLen is:" << MSG_UPDATE.Len << endl;
	cout << "MessageType is: UPDATE" << endl;
	cout << "Attributes Numbers is:" << MSG_UPDATE.AttributeNum << endl;
	for (int i = 0; i < MSG_UPDATE.AttributeNum; i++) {
		if (MSG_UPDATE.attributes[i].AttrFlag >> 7) {
			cout << "Unknow Attributes" << endl;
			continue;
		}
		int k = 0;
		int j = 0;
		switch (MSG_UPDATE.attributes[i].AttrType) {
		case DRRP_ID_ERMI1_UPDATE_WithdrawnRoutes:
			cout << "WithdrawnRoutes" << endl;
			for (k = 0; k < MSG_UPDATE.WRoutesNum; k++) {
				cout << "The number " << k + 1 << " Routes's AddFamily is:" << MSG_UPDATE.WithdrawnRoutes[k].AddFamily << endl;
				cout << "The number " << k + 1 << " Routes's AppProtocol is:" << MSG_UPDATE.WithdrawnRoutes[k].AppProtocol << endl;
				cout << "The number " << k + 1 << " Routes's AddLen is:" << MSG_UPDATE.WithdrawnRoutes[k].AddLen << endl;
				cout << "The number " << k + 1 << " Routes's Address is:";
				for (j = 0; j < MSG_UPDATE.WithdrawnRoutes[k].AddLen; j++) {
					cout << (char) MSG_UPDATE.WithdrawnRoutes[k].Address[j];
				}
				cout << endl;
			}
			break;
		case DRRP_ID_ERMI1_UPDATE_ReachableRoutes:
			cout << "ReachableRoutes" << endl;
			for (k = 0; k < MSG_UPDATE.RRoutesNum; k++) {
				cout << "The number " << k + 1 << " Routes's AddFamily is:" << MSG_UPDATE.ReachableRoutes[k].AddFamily << endl;
				cout << "The number " << k + 1 << " Routes's AppProtocol is:" << MSG_UPDATE.ReachableRoutes[k].AppProtocol << endl;
				cout << "The number " << k + 1 << " Routes's AddLen is:" << MSG_UPDATE.ReachableRoutes[k].AddLen << endl;
				cout << "The number " << k + 1 << " Routes's Address is:";
				for (j = 0; j < MSG_UPDATE.ReachableRoutes[k].AddLen; j++) {
					cout << (char) MSG_UPDATE.ReachableRoutes[k].Address[j];
				}
				cout << endl;
			}
			break;
		case DRRP_ID_ERMI1_UPDATE_NextHopServer:
			cout << "NextHopServer" << endl;
			cout << "AddLen is:" << MSG_UPDATE.NextHopServer.Length << endl;
			cout << "Address is:";
			for (k = 0; k < MSG_UPDATE.NextHopServer.Length; k++) {
				cout << (char) MSG_UPDATE.NextHopServer.Server[k];
			}
			cout << endl;
			break;
		case DRRP_ID_ERMI1_UPDATE_TotalBandwidth:
			cout << "TotalBandwidth: ";
			cout << MSG_UPDATE.totalbw << endl;
			break;
		case DRRP_ID_ERMI1_UPDATE_ServiceStatus:
			cout << "ServiceStatus:";
			cout << MSG_UPDATE.ServiceStatus << endl;
			break;
//TO DO add something ERMI 
		default:
			//TODO:未知代码参数处理
			break;
		}
	}
}
