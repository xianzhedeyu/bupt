#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <signal.h>
#include <arpa/inet.h>
#include <errno.h>
#include <iostream>
using namespace std;
int ConnectSock(int *pSock, int nPort, char * pAddr)
{
	struct sockaddr_in addrin;	
	long lAddr;	
	int nSock;	
	int nRetVal = -1;

	assert(pSock != NULL && nPort > 0 && pAddr != NULL);	
	assert((nSock = socket(AF_INET, SOCK_STREAM, 0)) > 0);

	memset(&addrin, 0, sizeof(addrin));	
	addrin.sin_family = AF_INET;
	addrin.sin_addr.s_addr = inet_addr(pAddr);
	addrin.sin_port = htons(nPort);

	if(nRetVal = connect(nSock, (struct sockaddr *)&addrin, sizeof(addrin)) == 0)
	{
		*pSock = nSock;		
		return 0;
	}
	else
	{
		close(nSock);
		return -1;
	}	
}

int rtsp_write(int fd, void *buffer, int nbytes)
{
	int n;
	n = write((int)fd, buffer, nbytes);
	return n;
}
int rtsp_read(int fd, void *buffer, int nbytes)
{
	int n;
	n=recv(fd,buffer,nbytes,0);
	return n;
}

int main()
{   
  
    int pSock;
    int nPort = 9999;
    char * pAddr = "127.0.0.1";
    char response[2048];
    int nread = -1;
//    string str = "GET_PARAMETER rtsp://172.2.2.2/123 RTSP/1.0\r\n"
//"CSeq: 130\r\n"
//"Require: ermi\r\n"
//"Content-length: 14\r\n"
//"Content-type: text/parameters\r\n\r\n"
//"session_list\r\n";
    char str[124]={0};
    //header
    str[0] = 0x00;
    str[1] = 0x7c;//120
    str[2] = 0x02;
    //att1
    str[3] = 0x00;//flag
    str[4] = 0x02;//code
    str[5] = 0x00;//length
    str[6] = 0x1d;//length 29
    str[7] = 0x80;//rtsp url 32769
    str[8] = 0x01;
    str[9] = 0x80;//ermi 32768
    str[10] = 0x00;
    str[11] = 0x00; //length
    str[12] = 0x17; // length 23
    str[13] = 'r';str[14] = 't';str[15] = 's';str[16] = 'p';str[17] = ':';str[18] = '/';
    str[19] = '/';str[20] = '1';str[21] = '9';str[22] = '2';str[23] = '.';str[24] = '1';
    str[25] = '6';str[26] = '8';str[27] = '.';str[28] = '0';str[29] = '.';str[30] = '2';
    str[31] = '/';str[32] = '1';str[33] = '2';str[34] = '3';str[35] = '4';
    str[36] = 0x00;//flag
    str[37] = 0x03;//nexthopserver
    str[38] = 0x00;//len
    str[39] = 0x11;//len:17
    str[40] = 0x00;//Next Hop Address Domain
    str[41] = 0x00;
    str[42] = 0x00;
    str[43] = 0x01;
    str[44] = 0x00;//length
    str[45] = 0x0b;//length 11
    str[46] = '1';str[47] = '1';str[48] = '1';str[49] = '1';str[50] = '1';str[51] = '1';
    str[52] = '1';str[53] = '1';str[54] = '1';str[55] = '1';str[56] = '1';
    str[57] = 0x00;
    str[58] = 0xea;//234
    str[59] = 0x00;//length
    str[60] = 0x08;//length 8
    str[61] = 0x00;//388000000 17206900
    str[62] = 0x00;
    str[63] = 0x00;
    str[64] = 0x00;
    str[65] = 0x17;
    str[66] = 0x20;
    str[67] = 0x69;
    str[68] = 0x00;
    str[69] = 0x00;//flag
    str[70] = 0xf0;//code port id
    str[71] = 0x00;
    str[72] = 0x04;//length
    str[73] = 0x12;//port id
    str[74] = 0x34;
    str[75] = 0x00;
    str[76] = 0x00;//channel id
    str[77] = 0x00;//flag
    str[78] = 0xf1;//241
    str[79] = 0x00;// 04
    str[80] = 0x04;// 04 length
    str[81] = 0x00;
    str[82] = 0x00;
    str[83] = 0x00;
    str[84] = 0x01;//service status
    str[85] = 0x00;
    str[86] = 0xf6;//code qamid
    str[87] = 0x00;//length
    str[88] = 0x02;
    str[89] = 0x00;//tsid
    str[90] = 0x17;
    str[91] = 0x00;//flag
    str[92] = 0xf8;
    str[93] = 0x00;
    str[94] = 0x08;//length
    str[95] = 0x20;//20C85580
    str[96] = 0xc8;
    str[97] = 0x55;
    str[98] = 0x80;
    str[99] = 0x01;
    str[100] = 0x10;///annex and ch
    str[101] = 0x20;//32
    str[102] = 0x04;//4
    str[103] = 0x00;//flag
    str[104] = 0xf9;
    str[105] = 0x00;
    str[106] = 0x11;//length
    str[107] = 0x00;
    str[108] = 0x00;
    str[109] = 0x00;
    str[110] = 0x18;//24
    str[111] = 0x00;
    str[112] = 0x0b;//11 length
    str[113] = '9';str[114] = '9';str[115] = '9';str[116] = '9';str[117] = '9';str[118] = '9';
    str[119] = '9';str[120] = '9';str[121] = '9';str[122] = '9';str[123] = '9';

    for(int g=0;g<124;g++)
    	printf("..%x..",str[g]);

    string msg = string(str);

    char* a = new char[msg.size()+1];
    cout << msg  << endl;
    strcpy(a,msg.c_str());
    cout<< a <<endl;

    ConnectSock(&pSock,nPort,pAddr); 
    cout << msg.size() << endl;
    cout << strlen(a) << endl;
    //rtsp_write(pSock,a, 122);
    send(pSock, str, 120, 0);
    nread=rtsp_read(pSock, response, 2048);

    for(int i=0;i<nread;i++)
    {
    	cout <<response[i];
    }

    return 0;
}
