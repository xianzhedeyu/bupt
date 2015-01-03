#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main() 
{
    char str[1024];
    char restr[1024];
    memset(str, 0x00, sizeof(str));
    memset(str, 0x00, sizeof(restr));

    char qam[256];
    memset(qam, 0x00, sizeof(qam));
    sprintf(qam, "MP2T/DVBC/UDP;unicast;client=%s;qam_name=%s;modulation=%s;bandwidth=%llu", "00AF123456DE", "qam1", "h.264", 2920263);
    sprintf(str, "SETUP rtsp://127.0.0.1:6606 RTSP/1.0\r\n"
            "CSeq:313\r\n"
            "Require:com.comcast.s1\r\n"
            "client_session_id:054321\r\n"
            "app_id:001\r\n"
            "app_type:1\r\n"
            "Transport:%s\r\n\r\n",
            qam);
    printf("%s", str);
    int sockfd;
    struct sockaddr_in servaddr;
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    memset(&servaddr, 0x00, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(6606);
    inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);
    connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    write(sockfd, str, strlen(str));
    read(sockfd, restr, 1024);
    printf("%s", restr);
    char ondemandsessionid[256];
    memset(ondemandsessionid, 0x00, 256);
    getOnDemandSessionId(restr, ondemandsessionid);

    sprintf(str, "TEARDOWN rtsp://127.0.0.1:6606 RTSP/1.0\r\n"
            "CSeq:314\r\n"
            "Require:com.comcast.s1\r\n"
            "Reason:200\r\n"
            "Session:0\r\n"
            "OnDemandSessionId:%s\r\n"
            "client_session_id:054321\r\n\r\n",
            ondemandsessionid);
    write(sockfd, str, strlen(str));
            
    return 0;
}
int getOnDemandSessionId(char *res, char *ondemandsessionid){
	char str[1024] = "";
	strncpy(str, res, strlen(res) - 4);
	
	char *line = NULL;
	char *header = NULL;
	char *header_val = NULL;
	char *ptr = NULL;
	char *ptr2 = NULL;
    char id[256];
    memset(id, 0x00, 256);
	
	line = strtok_r(str,"\r\n",&ptr);

	//按行解析剩余消息
	line = strtok_r(NULL,"\r\n",&ptr);
	while(line)
	{
		if (header = strtok_r(line,":",&ptr2))
        {
            if (strcmp(header, "OnDemandSessionId") == 0)
            {
                header_val = strtok_r(NULL,":",&ptr2);
                strcpy(id,header_val);
                break;
            }
        }	
		line = strtok_r(NULL,"\r\n",&ptr);
	}
	strcpy(ondemandsessionid, id);
	return 0;
}
