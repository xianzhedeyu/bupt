/*
版本:v1.0

 * 作者:王传华

 * 创建时间:2010-08-23

 *修改记录:

 *程序说明:
	S6 接入进程,进程管理进程，R6业务处理线程

 *输入参数: 无

 *输出参数: 无

 *返回值:无
*/

#include<errno.h>
#include"public_def.h"
#include"asm_task_control_module.h"
#include"asm_communication_module.h"
#include"asm_transaction.h"
#include <iostream>
//#include <occi.h>
using namespace std;
//using namespace oracle::occi;


int process_num=0;
int pthread_num=0;
pthread_mutex_t p_lock=PTHREAD_MUTEX_INITIALIZER;
SOCKET_DATA  S_pItem[ASM_MAXNUM];
process_info process_destory[ASM_MAXNUM];
pthread_queue data[ASM_MAXNUM];
extern int LVLDEBUG;
int lockfile(int fd,int type)
{
    struct flock f1;
    f1.l_type=type;
    f1.l_start=0;
    f1.l_whence=SEEK_SET;
    f1.l_len=0;
    return(fcntl(fd,F_SETLKW,&f1));
}
Sigfunc * Signal(int signo, Sigfunc *func)
{
    struct sigaction act,oact;
    act.sa_handler=func;
    sigemptyset(&act.sa_mask);
    act.sa_flags=0;
    if (signo==SIGALRM) {
#ifdef SA_INTERRUPT
        act.sa_flags|=SA_INTERRUPT;
#endif
    } else {
#ifdef SA_RESTART
        act.sa_flags|=SA_RESTART;
#endif
    }
    if (sigaction(signo,&act,&oact)<0)
        return(SIG_ERR);
    return(oact.sa_handler);
}
void child_exit(int sig)
{
    pid_t deadpid;
    int empty_i=0;
    int offset=sizeof(int);
    Signal(SIGCHLD,child_exit);
    while ((deadpid=waitpid(-1,NULL,WNOHANG))>0) {
        log(LVLDEBUG,SYS_INFO,"*********child process %d exit***********\n",deadpid);
        for (empty_i=0;empty_i<ASM_MAXNUM;empty_i++) {
            if (process_destory[empty_i].pid==deadpid) {
                memset(process_destory[empty_i].pid_queue_addr,0x00,sizeof(S_pItem)+offset);
                memset(&process_destory[empty_i],0x0,sizeof(process_info));
                break;
            }
        }
        process_num--;
    }
}
char *Create_mmap(char* mmapname,int memsize)
{
    int fd_in;
    fd_in=open(mmapname,O_CREAT|O_RDWR,00644 );
    lseek(fd_in,memsize,SEEK_SET);
    write(fd_in,"",1);
    char *mmap_buf = (char*)mmap(NULL,memsize,PROT_READ|PROT_WRITE,MAP_SHARED,fd_in,0);
    memset(mmap_buf,0x00,memsize);
    close(fd_in);
    return mmap_buf;
}
int Distroy_mmap(void* mmap_addr,int size)
{
    return munmap(mmap_addr,size);
}
/*S6 interproxy process*/

int interproxy(int lockfd)
{
    int pSock,nPort,sd=0,readlen=0,ret=0;
    int find_empty=0,find_tmp=0;
    int find_data_flag=0;
    Queue_info msg;
    memset(&msg,0x00,sizeof(msg));
    SOCKET_DATA pItem;
    SOCKET_DATA *mmap_buf;
    char * mmap_ptr;
    memset(S_pItem,0x00,sizeof(S_pItem));

    char mmapname[128]="";
    int mmapfile_num=0;
    char mmapfile[64]="Queue_file";

    int offset=sizeof(int);
    int mmapsize=sizeof(S_pItem)+offset;
    /*
    *共享内存区的管理列表
    *子进程无法访问,无需加锁
    */
    mmap_queue_info queue_info[ASM_MAXNUM];
    memset(&queue_info,0x00,sizeof(queue_info));

    int mmap_data_flag=1;

    /****

    *共享内存区头部偏移量
    *0代表可用(空闲)
    *1代表有子进程在占用
    *2代表有数据可用，防止进程提前退出

    ****/
    char buff[1024]="";
    for (mmapfile_num=0;mmapfile_num<ASM_MAXNUM;mmapfile_num++) {
        sprintf(mmapname,"%s/%s%d",MMAPDIR,mmapfile,mmapfile_num);

        queue_info[mmapfile_num].mmap_queue=Create_mmap(mmapname,mmapsize);
    }


    int fd[2],sock_sd[2],pid;
    pipe(fd);
    socketpair(AF_LOCAL,SOCK_STREAM,0,sock_sd);

    pid=fork();
    if (pid==0) {
        close(fd[1]);//靠靠
        close(sock_sd[1]);
	fprintf(stderr, "process manager start\n");
        process_manager(fd[0],sock_sd[0],lockfd);
        exit(0);
    }
    close(fd[0]);
    close(sock_sd[0]);
    int fw=fd[1];
    int sw=sock_sd[1];

    /*use enable*/
    queue_info[0].flag=1;
    mmap_ptr=queue_info[0].mmap_queue;
    mmap_buf=(SOCKET_DATA*)(queue_info[0].mmap_queue+offset);
    memcpy(mmap_ptr,&mmap_data_flag,offset);
    /*mmap_buf ptr*/
    msg.Queue_addr=mmap_ptr;
    /*pipe ipc*/

    /*
    	write(fw,(char *)&msg,sizeof(msg));
    	write_fd(sw,(void*)&ret,sizeof(int),sd);

    */
    nPort=ASM_PORT;
    CreateSock(&pSock,nPort);
    int frist=1;

    while (1) {
        memset(buff,0x00,sizeof(buff));
        memset(&pItem,0x00,sizeof(pItem));
        log(LVLDEBUG,SYS_INFO,"++++++++++be accept\n");
        AcceptSock(&sd,pSock);
        log(LVLDEBUG,SYS_INFO,"==========af accept\n");
        readlen=rtsp_read(sd,buff,1024);

        fprintf(stderr,"**********af read buf:%s , len=%d\n",buff,readlen);

        log(LVLDEBUG,SYS_INFO,"**********af read buf:%s , len=%d\n",buff,readlen);
        if (readlen<=-1) {
            log(LVLDEBUG,SYS_INFO,"interproxy read error");
            sleep(1);
            continue;
        }
        pItem.data_len=readlen;
        memcpy(pItem.data,buff,readlen);

        write_lock(lockfd);

        ret=PushToQueue(&pItem,mmap_buf);

        memcpy(&find_data_flag,mmap_ptr,offset);
        /*
        *如果队列满，进程一定存在
        *如果进程死亡，队列一定为空
        */
        if (find_data_flag==0||frist) { /*进程已经死亡*/
            frist=0;
            log(LVLDEBUG,SYS_INFO,"*******************************\n");
            write(fw,(char *)&msg,sizeof(msg));
        }
        /*插入队列*/
        write_fd(sw,(void*)&ret,sizeof(int),sd);
        find_data_flag=2;
        memcpy(mmap_ptr,&find_data_flag,offset);/*预防进程提前死亡*/
        write_unlock(lockfd);

        if (ret==-1) { /*节点内容大于100,检查节点管理列表*/
            log(LVLDEBUG,SYS_INFO,"***full***\n");
            for (find_empty=0;find_empty<ASM_MAXNUM;find_empty++) {
                if (queue_info[find_empty].flag==0) {
                    log(LVLDEBUG,SYS_INFO,"***find@@@%d\n",find_empty);
                    queue_info[find_empty].flag=1;
                    mmap_ptr=queue_info[find_empty].mmap_queue;
                    mmap_buf=(SOCKET_DATA*)(queue_info[find_empty].mmap_queue+offset);
                    memcpy(mmap_ptr,&mmap_data_flag,offset);
                    memset(&msg,0x00,sizeof(msg));
                    msg.Queue_addr=mmap_ptr;

                    write_lock(lockfd);
                    ret=PushToQueue(&pItem,mmap_buf);
                    write(fw,(char *)&msg,sizeof(msg));
                    write_fd(sw,(void*)&ret,sizeof(int),sd);
                    write_unlock(lockfd);

                    break;

                }

                /*节点用过每20个，回头检查一下内存区标识*/
                if (find_empty%20==0) {
                    for (find_tmp=0;find_tmp<ASM_MAXNUM;find_tmp++) {
                        memcpy(&find_data_flag,queue_info[find_tmp].mmap_queue,offset);
                        if (find_data_flag==0)
                            queue_info[find_tmp].flag=0;
                    }
                }
                /*所有节点满，循环检查，直到空节点被发现*/
                if (find_empty>=ASM_MAXNUM-1) {
                    log(LVLDEBUG,SYS_INFO,"***all full***\n");
                    sleep(1);
                    find_empty=-1;
                }
            }
        }
        close(sd);
    }
}
void *pthread_SM(void *arg)
{
    int ret,sd;
    pthread_args *p_args = (pthread_args*)arg;
    int i=p_args->i;
    SOCKET_DATA pItem;
    memset(&pItem,0x00,sizeof(pItem));
    pthread_mutex_lock(&p_lock);
    sd=data[i].sd;
    pItem.data_len=data[i].data_len;
    memcpy(&pItem.data,data[i].data,pItem.data_len);
    data[i].data_len=0;
    memset(data[i].data,0x00,sizeof(data[i].data));
    pthread_mutex_unlock(&p_lock);

    fprintf(stderr, "********pthread read buf=%s, buflen=%d\n",pItem.data,pItem.data_len);

    log(LVLDEBUG,SYS_INFO,"********pthread read buf=%s, buflen=%d\n",pItem.data,pItem.data_len);

    ret=pthread_TP(pItem.data,pItem.data_len,sd);

    close(sd);
    pthread_mutex_lock(&p_lock);
    data[i].data_flag=0;
    pthread_num--;
    pthread_mutex_unlock(&p_lock);
    log(LVLDEBUG,SYS_INFO,"pthread exit\n");
    pthread_exit(0);
}
int process_manager(int fr,int sr,int lockfd)
{
    int datalen=0,ret=0,len=0,pid=0;
    int mmap_data_flag=0;
    int find_data_flag=1;
    int offset=sizeof(int);
    int c=0;
    Queue_info msg;
    SOCKET_DATA pItem;
    SOCKET_DATA *mmap_buf;
    char *mmap_ptr;
    memset(&pItem,0x00,sizeof(SOCKET_DATA));
    memset(&data,0x00,sizeof(data));

    int i,sd;
    len=sizeof(msg);
    pthread_t threadid;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

    struct strrecvfd recvfd;
    Signal(SIGCHLD,child_exit);
    int pipefd[2];
    int empty_i=0;
    memset(process_destory,0x0,sizeof(process_destory));
    while (1) {
        memset(&msg,0x00,len);
        /*pipe ipc*/
        ret=read(fr,(char *)&msg,len);
        /*mmap ptr*/
        mmap_ptr=msg.Queue_addr;
        mmap_buf=(SOCKET_DATA*)(mmap_ptr+offset);
        if (ret<=0) {
            exit(0);
            /*异常处理*/
        }
        if (process_num>ASM_MAXNUM) {
            log(LVLDEBUG,SYS_INFO,"Process num > %d\n",ASM_MAXNUM);
            /*不会产生*/
            /*异常处理*/
            //exit(0);
        }
        process_num++;
        pipe(pipefd);
        while (1) {
            pid=fork();
            if (pid<0) {
                log(LVLDEBUG,SYS_INFO,"Fork error\n");
                sleep(1);
                continue;
            } else
                break;
        }
        if (pid) {
            for (empty_i=0;empty_i<ASM_MAXNUM;empty_i++) {
                if (process_destory[empty_i].pid==0) {
                    process_destory[empty_i].pid=pid;
                    process_destory[empty_i].pid_queue_addr=mmap_ptr;
                    break;
                }
            }

            close(pipefd[1]);
            /*阻塞、等待子进程关闭 sr描述符*/
            ret=read(pipefd[0],NULL,1); /*收到信号后，停止阻塞，返回while循环创建新的进程*/
            close(pipefd[0]);
        } 
        else if (!pid) {

            fprintf(stderr, "child process %d runing\n",getpid());

            log(LVLDEBUG,SYS_INFO,"child process %d runing\n",getpid());
            close(pipefd[0]);
            close(fr);
            
	        //fprintf(stderr, "db pool create....\n");
            ////创建数据库线程池
            //const string userName = ORA_NAME;
        	//const string password = ORA_PWD;
        	//const string connectString = ORA_CONN;
        	//unsigned int maxConn = ORA_MAXCONN;
        	//unsigned int minConn = ORA_MINCONN;
        	//unsigned int incrConn = ORA_INCRCONN;
        	//Environment *env = Environment::createEnvironment(Environment::DEFAULT);
        	//StatelessConnectionPool *connPool = env->createStatelessConnectionPool(userName,password,connectString,\
            //                                    maxConn,minConn,incrConn,StatelessConnectionPool::HOMOGENEOUS);		
			pthread_args *p_args =(pthread_args *)malloc(sizeof(pthread_args));
			//p_args->connPool = connPool;
			p_args->connPool = NULL;
	fprintf(stderr, "db pool done....\n");
                       
            while (1) {
                if (pthread_num>ASM_MAXNUM) { /*异常，不会产生*/
                    log(LVLDEBUG,SYS_INFO,"Pthread full\n");
                    usleep(1000);
                    continue;
                }
                memset(&pItem,0x00,sizeof(pItem));
                find_data_flag=1;
                if (c!=-1) {
		fprintf(stderr, "read_sd\n");
                    read_fd(sr,&c,sizeof(int),&sd);
                }
                if (c==-1) { /*full*/
                	/*当前共享内存满时，通过pipe给父进程发送空信息*/
                    write(pipefd[1],"",1);/*可以省，直接用pipe破碎*/
                    close(pipefd[1]);
                    close(sr);
                    if (ret!=-1) {
                        write_lock(lockfd);
                        ret=PopFromQueue(c,&pItem,mmap_buf);
                        memcpy(mmap_ptr,&find_data_flag,offset);/*在自己负责的队列上做标记*/
                        write_unlock(lockfd);
                        ret=-1;
                    }
                    /*
                    	check pthread list & find timeout
                    	pthread_cancel();
                    */
                } else {
                    write_lock(lockfd);
                    ret=PopFromQueue(c,&pItem,mmap_buf);
                    /*
                     *数据已读
                     */
                    memcpy(mmap_ptr,&find_data_flag,offset);/*在自己负责的队列上做标记*/
                    write_unlock(lockfd);
                }
                if (ret!=-1) { /*have data*/
                    for (i=0;i<ASM_MAXNUM;i++) {
                        /*
                        *p_flag=0&&d_flag=0 :read enable
                        *p_flag=1&&d_flag=1 :pthread run
                        *p_flag=1&&d_flag=0 :pthread done
                         */
                        if (data[i].pthread_flag==0&&data[i].data_flag==0) {
                            pthread_mutex_lock(&p_lock);
                            data[i].data_flag=1;
                            data[i].pthread_flag=1;

                            data[i].data_id=ret;
                            data[i].sd=sd;
                            data[i].data_len=pItem.data_len;
                            memcpy(data[i].data,pItem.data,pItem.data_len);

                            pthread_num++;
                            pthread_mutex_unlock(&p_lock);
                            while (1) {
                            	p_args->i = i;
                                ret=pthread_create(&threadid,&attr,pthread_SM,(void*)p_args);
                                if (ret==0)break;
                                sleep(1);
                            }
                            break;
                        }
                    }
                } else { /*no data*/
                    if (pthread_num==0) { /*no pthread*/
                        /*
                         *退出前检查
                         */
                        write_lock(lockfd);
                        memcpy(&find_data_flag,mmap_ptr,offset);
                        if (find_data_flag==2) {
                            write_unlock(lockfd);
                            continue;
                        }
                        /*
                         *标识为空节点
                         */
                        memset(mmap_ptr,0x00,sizeof(S_pItem)+offset);
                        //memcpy(mmap_buf,&mmap_data_flag,offset);/*在自己负责的队列上做标记*/
                        write_unlock(lockfd);
                        close(lockfd);
                        //关闭数据库连接池
                        free(p_args);
                        //env->terminateStatelessConnectionPool(connPool);
        				//Environment::terminateEnvironment(env);
                        log(LVLDEBUG,SYS_INFO,"%d normal exit\n",getpid());
                        exit(0);
                    }
                }
                usleep(1);
            }
        }
    }
}
