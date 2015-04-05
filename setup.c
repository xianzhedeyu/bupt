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
    int n = read(sockfd, restr, 1024);
    printf("\n%s\n", restr);
    close(sockfd);
}
