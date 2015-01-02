#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "public_def.h"
#include "as_rtsp_r8_msg_process.h"

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
            if((n = read(connfd, str, 1024)) > 0)
            {
                printf("setup send to as msg:%slen:%d\n", str, n);
                R8_SETUP_MSG msg;
                memset(&msg, 0x00, sizeof(msg));
                rtsp_r8_setup_msg_parse(str, &msg);
                /*
                 * 部署应用
                 */
                printf("application id:%s\n", msg.app_id);
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
