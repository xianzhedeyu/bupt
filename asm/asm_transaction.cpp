#include"asm_communication_module.h"
#include"asm_transaction.h"
#include"asmlog.h"
#include "asm_rtsp_public_function.h"
#include <sys/time.h>
#include "asm_rtsp_s7_msg_process.h"
#include "asm_rtsp_r8_msg_process.h"
//#include <hiredis/hiredis.h>
#include <mysql.h>
#include "hash.h"
#include "semaphore.h"

extern int LVLDEBUG;
int request_bw=0xFFFFFFF;
int pthread_TP(char * Asm_msg, int msg_len, int asm_sd) {
    int ret = -1;
    char* msg;
	msg = Asm_msg;
	//ret = rtsp_Asm_response_parse(msg);
    ret = rtsp_get_msg_type(msg);
	if (ret == RTSP_ID_S1_SETUP){
		fprintf(stderr, "------ASM SETUP....\n");
		Asm_Setup(Asm_msg, msg_len, asm_sd);
	} else if (ret == RTSP_ID_S1_TEARDOWN) {
		fprintf(stderr, "------ASM TEARDOWN....\n");
		Asm_Teardown(Asm_msg, msg_len, asm_sd);
	} 
    //else if (ret=="GET_PARAMETER"){
	//	fprintf(stderr, "------ASM GETPARAMETER....\n");
	//	Asm_GetParam(Asm_msg, msg_len, asm_sd);
	//} else {
	//	write(asm_sd, "TYPE ERROR", 20);
	//}
	return 0;
}

int Asm_Setup(char * Asm_msg, int msg_len, int asm_sd) {
	log(LVLDEBUG,SYS_INFO,"sm_setup msg :%s len:%d\n",Asm_msg,msg_len);
	char as_addr[40] = "";
	int as_port = 0;
	char sendbuf[1024];
	char recvbuf[1024];
	int as_sd;
	int buflen = 0;
	int i = 0, ret = 0;
	char* buf = Asm_msg;
	INT64 asm2sm_session = 0;
    MYSQL *mysql;
    if((mysql = mysql_init(NULL)) == NULL) {
        fprintf(stderr, "Cannot initialize MySQL\n");
        return -1;
    }
    //redisContext *c;
    //redisReply *reply;
    //const char *hostname = "127.0.0.1";
    //int port = 6379;
    //struct timeval timeout = {1, 500000};
    //c = redisConnectWithTimeout(hostname, port, timeout);

    R8_SETUP_MSG r8_setup_msg;
    R8_SETUP_RES r8_setup_res;
    S7_SETUP_MSG s7_setup_msg;
    S7_SETUP_RES s7_setup_res;

    //连接数据库
    const char *dbhost = "127.0.0.1";
    const char *user = "root";
    const char *passwd = "123456";
    unsigned int port = 0;
    const char *database = "cloud";
    const char *sock= NULL;
    unsigned long flag = 0;
    if(!mysql_real_connect(mysql, dbhost, user, passwd, database, port, sock, flag)) {
            fprintf(stderr, "%d: %s\n", mysql_errno(mysql), mysql_error(mysql));
            return -1;
     }

    INT64 asm2as_cseq = 0;
    struct timeval now_tmp;
    gettimeofday(&now_tmp, 0);
    srand((now_tmp.tv_sec * 1000) + (now_tmp.tv_usec / 1000));
    asm2sm_session = 1 + (int) (10.0 * rand() / (100000 + 1.0));
    memset(&s7_setup_msg, 0x00, sizeof(S7_SETUP_MSG));
    fprintf(stderr, "parse s7 message...\n");
    rtsp_s7_setup_msg_parse(Asm_msg, &s7_setup_msg);

    /*
     * 选择应用服务器
     */
    int sem_id = semget((key_t)1234, 1, 0666 | IPC_CREAT);
    semaphore_p(sem_id);
    int fd = shm_open("asinfo", O_RDONLY, 0);
    struct stat stat;
    fstat(fd, &stat);
    ASNode* list = (ASNode *)mmap(NULL, stat.st_size, PROT_READ, MAP_SHARED, fd, 0);
    close(fd);
    int fd_min = shm_open("min", O_RDONLY, 0);
    fstat(fd_min, &stat);
    int *min = (int *)mmap(NULL, stat.st_size, PROT_READ, MAP_SHARED, fd, 0);
    
    memset(&r8_setup_msg, 0x00, sizeof(R8_SETUP_MSG));
    strcpy(r8_setup_msg.as_ip, "127.0.0.1");
    //strcpy(r8_setup_msg.as_ip, list[min].ip);
    semaphore_v(sem_id);
    r8_setup_msg.as_port = AS_PORT;
    r8_setup_msg.cseq = asm2as_cseq;
    strcpy(r8_setup_msg.require, RTSP_R8_REQUIRE);
    strcpy(r8_setup_msg.session_group, s7_setup_msg.session_group);
    strcpy(r8_setup_msg.ondemandsessionid, s7_setup_msg.ondemandsessionid);
    strcpy(r8_setup_msg.policy, s7_setup_msg.policy);
    strcpy(r8_setup_msg.app_id, s7_setup_msg.app_id);
    r8_setup_msg.app_type = s7_setup_msg.app_type;
    strcpy(r8_setup_msg.ss.client, s7_setup_msg.ss.client);
    strcpy(r8_setup_msg.ss.destination, s7_setup_msg.ss.destination);
    r8_setup_msg.ss.client_port = s7_setup_msg.ss.client_port;
    r8_setup_msg.ss.bandwidth = s7_setup_msg.ss.bandwidth;

    fprintf(stderr, "connect to as ...\n");
    //ret = ConnectSock(&as_sd, 7778, "127.0.0.1");
    as_sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in servaddr;
    memset(&servaddr, 0x00, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(AS_PORT);
    //inet_pton(AF_INET, r8_setup_msg.as_ip, &servaddr.sin_addr);
    inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);
    ret = connect(as_sd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    if(ret == -1) {
        fprintf(stderr, "connect to as error...\n");
        return -1;
    }

    fprintf(stderr, "connect to as sucess...\n");
    memset(sendbuf, 0x00, 1024);
    fprintf(stderr, "msg to r8 encode\n");
    rtsp_r8_setup_msg_encode(r8_setup_msg, sendbuf);
    fprintf(stderr, "send msg to r8\n");
    ret = rtsp_write(as_sd, sendbuf, strlen(sendbuf) + 1);
    log(LVLDEBUG, SYS_INFO, "setup send to as msg:\n%slen:%d\n", sendbuf, ret);

    memset(recvbuf, 0x00, 1024);
    ret = rtsp_read(as_sd, recvbuf, 1024);
    fprintf(stderr, "read msg from r8 done\n");
    printf("%s", recvbuf);
    log(LVLDEBUG, SYS_INFO, "setup recv from as res:%s, len:%d\n", recvbuf, ret);
    //char key[41];
    //memset(key, 0x00, 41);
    //sprintf(key, "%sasm", s7_setup_msg.ondemandsessionid);
    //char value[50];
    //memset(value, 0x00, 50);
    //sprintf(value, "%llu", r8_setup_res.session);
    //reply = (redisReply*)redisCommand(c, "SET %s %s", key, value);
    //freeReplyObject(reply);
    //redisFree(c);

    memset(&r8_setup_res, 0x00, sizeof(R8_SETUP_RES));
    rtsp_r8_setup_res_parse(recvbuf, &r8_setup_res);
    fprintf(stderr, "parse msg from r8 done\n");

    //将与AS的会话信息入库
    char sql_cmd[512];
    memset(sql_cmd, 512, 0x00);
    snprintf(sql_cmd, 512, "insert into ASM_R8(ondemandsession, session, session_group, policy, app_id, app_type, client, destination, client_port, as_ip, as_upPort, as_downPort, status) values('%s', %llu, '%s', '%s', '%s', %d, '%s', '%s', %d, '%s', %d, %d, '%s')", r8_setup_msg.ondemandsessionid, r8_setup_res.session, r8_setup_msg.session_group, r8_setup_msg.policy, r8_setup_msg.app_id, r8_setup_msg.app_type, r8_setup_msg.ss.client, r8_setup_msg.ss.destination, r8_setup_msg.ss.client_port, r8_setup_res.as.ip, r8_setup_res.as.upPort, r8_setup_res.as.downPort, "SETUP");
    mysql_query(mysql, sql_cmd);
    mysql_close(mysql);

    memset(&s7_setup_res, 0x00, sizeof(S7_SETUP_RES));
    s7_setup_res.cseq = s7_setup_msg.cseq;
    s7_setup_res.err_code = r8_setup_res.err_code;
    strcpy(s7_setup_res.protocol, r8_setup_res.protocol);
    s7_setup_res.session = asm2sm_session;
    strcpy(s7_setup_res.ondemandsessionid, r8_setup_res.ondemandsessionid);
    strcpy(s7_setup_res.ss.client, r8_setup_res.ss.client);
    strcpy(s7_setup_res.ss.destination, r8_setup_res.ss.destination);
    s7_setup_res.ss.client_port = r8_setup_res.ss.client_port;
    s7_setup_res.ss.bandwidth = r8_setup_msg.ss.bandwidth;
    strcpy(s7_setup_res.as.ip, r8_setup_res.as.ip);
    s7_setup_res.as.downPort = r8_setup_res.as.downPort;
    s7_setup_res.as.upPort = r8_setup_res.as.upPort;

    memset(sendbuf, 0x00, 1024);
    rtsp_s7_setup_res_encode(s7_setup_res, sendbuf);
    
    ret = rtsp_write(asm_sd, sendbuf, strlen(sendbuf) + 1);
    log(LVLDEBUG, SYS_INFO, "setup response send to sm msg:%s, len:%d\n", sendbuf, ret);

    return 0;
}


int Asm_Teardown(char * Asm_msg, int msg_len, int asm_sd) {
	char as_addr[40] = "";
	int as_port = 0;
	char sendbuf[1024];
	char recvbuf[1024];
	int as_sd = 0;
	int buflen = 0;
	int i = 0, ret = 0;
	char* buf = Asm_msg;
	INT64 as2asm_session = 0;
    int asm2as_cseq = 0;
    char sql_cmd[256];

    //redisContext *c;
    //redisReply *reply;
    //const char *hostname = "127.0.0.1";
    //int port = 6379;
    //struct timeval timeout = {1, 500000};
    //c = redisConnectWithTimeout(hostname, port, timeout);

    MYSQL *mysql;
    MYSQL_RES *result;
    MYSQL_ROW row;
    //连接数据库
    if((mysql = mysql_init(NULL)) == NULL)
    {
        fprintf(stderr, "Cannot initialize MySQL\n");
        return -1;
    }
    const char *dbhost = "127.0.0.1";
    const char *user = "root";
    const char *passwd = "123456";
    unsigned int port = 0;
    const char *database = "cloud";
    const char *sock = NULL;
    unsigned long flag = 0;
    if(!mysql_real_connect(mysql, dbhost, user, passwd, database, port, sock, flag)) {
            fprintf(stderr, "%d: %s\n", mysql_errno(mysql), mysql_error(mysql));
            return -1;
     }

    S7_TEARDOWN_MSG s7_teardown_msg;
    S7_TEARDOWN_RES s7_teardown_res;
    R8_TEARDOWN_MSG r8_teardown_msg;
    R8_TEARDOWN_RES r8_teardown_res;

    memset(recvbuf, 0x00, 1024);
    strcpy(recvbuf, Asm_msg);
    rtsp_s7_teardown_msg_parse(recvbuf, &s7_teardown_msg);

    //char key[41];
    //memset(key, 0x00, 41);
    //sprintf(key, "%sasm", s7_teardown_msg.ondemandsessionid);
    //reply = (redisReply *)redisCommand(c, "GET %s", key);
    //as2asm_session = atoi(reply->str);
    //freeReplyObject(reply);
    snprintf(sql_cmd, 256, "select session, as_ip from ASM_R8 where ondemandsession='%s' and status='SETUP'", s7_teardown_msg.ondemandsessionid);
    printf("%s\n", sql_cmd);
    mysql_query(mysql, sql_cmd);
    result = mysql_store_result(mysql);
    int num_fields = mysql_num_fields(result);
    while((row = mysql_fetch_row(result))) {
        as2asm_session = atol(row[0]);
        strcpy(as_addr, row[1]);
    }
    //strcpy(r8_teardown_msg.as_ip, as_addr);
    strcpy(r8_teardown_msg.as_ip, "127.0.0.1");
    r8_teardown_msg.as_port = AS_PORT;
    r8_teardown_msg.cseq = asm2as_cseq;
    strcpy(r8_teardown_msg.require, "com.comcast.rtsp.r8");
    r8_teardown_msg.reason = s7_teardown_msg.reason;
    r8_teardown_msg.session = as2asm_session;
    strcpy(r8_teardown_msg.ondemandsessionid, s7_teardown_msg.ondemandsessionid);

    memset(sendbuf, 0x00, 1024);
    rtsp_r8_teardown_msg_encode(r8_teardown_msg, sendbuf);

    as_sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in servaddr;
    memset(&servaddr, 0x00, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(AS_PORT);
    //inet_pton(AF_INET, r8_setup_msg.as_ip, &servaddr.sin_addr);
    inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);
    ret = connect(as_sd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    if(ret == -1) {
        fprintf(stderr, "connect to as error...\n");
        return -1;
    }
    ret = rtsp_write(as_sd, sendbuf, strlen(sendbuf) + 1);
    log(LVLDEBUG, SYS_INFO, "teardown send to as msg\n%slen:%d\n", sendbuf, ret);
    printf("teardown send to as msg\n%s, len:%d\n", sendbuf, ret);
    memset(recvbuf, 0x00, 1024);
    ret = rtsp_read(as_sd, recvbuf, 1024);
    log(LVLDEBUG, SYS_INFO, "teardown res recv from as res:\n%slen%d\n", recvbuf, ret);
    printf("teardown res recv from as res:\n%s, len%d\n", recvbuf, ret);
    printf("%s", recvbuf);
    rtsp_r8_teardown_res_parse(recvbuf, &r8_teardown_res);
    
    s7_teardown_res.err_code = 200;
    s7_teardown_res.cseq = s7_teardown_msg.cseq;
    s7_teardown_res.session = s7_teardown_msg.session;
    strcpy(s7_teardown_res.ondemandsessionid, s7_teardown_msg.ondemandsessionid);
    memset(sendbuf, 0x00, 1024);
    rtsp_s7_teardown_res_encode(s7_teardown_res, sendbuf);
    rtsp_write(asm_sd, sendbuf, strlen(sendbuf) + 1);
    log(LVLDEBUG, SYS_INFO, "teardown response send to sm msg:%s, len:%d\n", sendbuf, ret);
    memset(sql_cmd, 0x00, 256);
    snprintf(sql_cmd, 256, "update ASM_R8 set status='TEARDOWN' where ondemandsession='%s'", s7_teardown_msg.ondemandsessionid);
    mysql_query(mysql, sql_cmd);
    mysql_close(mysql);

    close(as_sd);
    return 0;
}
