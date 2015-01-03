#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/time.h>
#include "public_def.h"
#include "as_rtsp_r8_msg_process.h"
#include "as_rtsp_public_function.h"

int main()
{
    pid_t pid;
    int listenfd, connfd;
    
    listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in servaddr;
    struct sockaddr_in cliaddr;
    socklen_t clilen;

    memset(&servaddr, 0x00, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(AS_PORT);
    bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    listen(listenfd, 5);
    while(1)
    {
        clilen = sizeof(cliaddr);
        connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &clilen);

        if((pid = fork()) == 0)
        {
            close(listenfd);
            char str[1024];
            memset(str, 0x00, 1024);
            int n = 0;
            int ret = 0;
            if((n = read(connfd, str, 1024)) > 0)
            {
                if(rtsp_get_msg_type(str) == RTSP_ID_S1_SETUP)
                {
                    printf("setup send to as msg:\n%slen:%d\n", str, n);
                    R8_SETUP_MSG msg;
                    memset(&msg, 0x00, sizeof(msg));
                    rtsp_r8_setup_msg_parse(str, &msg);
                    /*
                     * 部署应用
                     */
                    R8_SETUP_RES res;
                    memset(&res, 0x00, sizeof(res));
                    res.err_code = 200;
                    res.cseq = msg.cseq;
                    strcpy(res.protocol, "RTSP");
                    INT64 as2asm_session = 0;
                    struct timeval now_tmp;
                    gettimeofday(&now_tmp, 0);
                    srand((now_tmp.tv_sec * 1000) + (now_tmp.tv_usec / 1000));
                    as2asm_session = 1 + (int) (10.0 * rand() / (100000 + 1.0));;
                    res.session = as2asm_session;
                    strcpy(res.ondemandsessionid, msg.ondemandsessionid);
                    strcpy(res.ss.client, msg.ss.client);
                    strcpy(res.ss.destination, msg.ss.destination);
                    res.ss.client_port = msg.ss.client_port;
                    res.ss.bandwidth = msg.ss.bandwidth;
                    strcpy(res.as.ip, "127.0.0.1");
                    res.as.downPort = 5000;
                    res.as.upPort = 6000;
                    char resmsg[1024];
                    memset(resmsg, 0x00, 1024);
                    rtsp_r8_setup_res_encode(res, resmsg);
                    write(connfd, resmsg, strlen(resmsg) + 1);
                }
                else if(rtsp_get_msg_type(str) == RTSP_ID_S1_TEARDOWN) {
                    printf("teardown send to as msg:\n%slen:%d\n", str, n);
                    R8_TEARDOWN_MSG msg;
                    R8_TEARDOWN_RES res;
                    memset(&msg, 0x00, sizeof(msg));
                    rtsp_r8_teardown_msg_parse(str, &msg);
                    /*
                     * 释放资源
                     */
                    res.err_code = 200;
                    res.cseq = msg.cseq;
                    res.session = msg.session;
                    strcpy(res.ondemandsessionid, msg.ondemandsessionid);
                    char resmsg[1024];
                    memset(resmsg, 0x00, 1024);
                    rtsp_r8_teardown_res_encode(res, resmsg);
                    write(connfd, resmsg, strlen(resmsg) + 1);
                }
                close(connfd);
                exit(0);
            }
            else{
                fprintf(stderr, "read from r8 error~\n");
                exit(1);
            }
        }
        close(connfd);
    }
    return 0;
}
