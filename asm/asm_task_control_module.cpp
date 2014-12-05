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
#include"erm_task_control_module.h"
#include"erm_commnu_module.h"
#include"erm_transaction.h"
#include"ermi_transaction.h"
#include"TThread.h"
#include"TThreadPool.h"
#include"R6Job.h"
#include "ErmiJob.h"
int process_num = 0;
int pthread_num = 0;
pthread_mutex_t p_lock = PTHREAD_MUTEX_INITIALIZER;
SOCKET_DATA S_pItem[ERM_MAXNUM];
process_info process_destory[ERM_MAXNUM];
pthread_queue data[ERM_MAXNUM];
extern int LVLDEBUG;

using namespace ThreadPool;

int lockfile(int fd, int type) {
	struct flock f1;
	f1.l_type = type;
	f1.l_start = 0;
	f1.l_whence = SEEK_SET;
	f1.l_len = 0;
	return (fcntl(fd, F_SETLKW, &f1));
}
Sigfunc * Signal(int signo, Sigfunc *func) {
	struct sigaction act, oact;
	act.sa_handler = func;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	if (signo == SIGALRM) {
#ifdef SA_INTERRUPT
		act.sa_flags |= SA_INTERRUPT;
#endif
	} else {
#ifdef SA_RESTART
		act.sa_flags |= SA_RESTART;
#endif
	}
	if (sigaction(signo, &act, &oact) < 0)
		return (SIG_ERR);
	return (oact.sa_handler);
}
void child_exit(int sig) {
	pid_t deadpid;
	int empty_i = 0;
	int offset = sizeof(int);
	Signal(SIGCHLD, child_exit);
	while ((deadpid = waitpid(-1, NULL, WNOHANG)) > 0) {
		log(LVLDEBUG,SYS_INFO,"*********child process %d exit***********\n",deadpid);
		for (empty_i = 0; empty_i < ERM_MAXNUM; empty_i++) {
			if (process_destory[empty_i].pid == deadpid) {
				memset(process_destory[empty_i].pid_queue_addr, 0x00, sizeof(S_pItem) + offset);
				memset(&process_destory[empty_i], 0x0, sizeof(process_info));
				break;
			}
		}
		process_num--;
	}
}
char *Create_mmap(char* mmapname, int memsize) {
	int fd_in;
	fd_in = open(mmapname, O_CREAT | O_RDWR, 00644);
	lseek(fd_in, memsize, SEEK_SET);
	write(fd_in, "", 1);
	char *mmap_buf = (char*) mmap(NULL, memsize, PROT_READ | PROT_WRITE, MAP_SHARED, fd_in, 0);
	memset(mmap_buf, 0x00, memsize);
	close(fd_in);
	return mmap_buf;
}
int Distroy_mmap(void* mmap_addr, int size) {
	return munmap(mmap_addr, size);
}
/*S6 interproxy process*/
/*
 * This function maps ERM_MAXNUM files to a memory, and accomplishes manage the memory.
 * It receives the connection from the client and delivers the descriptor to the sub-process which handle the data.
 * This function manage the memory. it creates a pipe and a unix domain socket between the father and son process.
 * Unix domain socket is used to deliver the descriptor and the pipe is used to deliver information between them.
 * Here we use three data structure: Queue_info, SOCKET_DATA, mmap_queue_info to accomplish the management of the
 * mapped memory.
 * We initialize a memory which size is ERM_MAXNUM * ERM_MAXNUM, and they were split into ERM_MAXNUM blocks, thus, each
 * block was ERM_MAXNUM byte.
 * When a connection arrives,
 *
 */

int interproxy(int lockfd) {
	int pSock, nPort, sd = 0, readlen = 0, ret = 0;
	int find_empty = 0, find_tmp = 0;
	int find_data_flag = 0;
	Queue_info msg;
	memset(&msg, 0x00, sizeof(msg));
	SOCKET_DATA pItem;
	SOCKET_DATA *mmap_buf;/*数据区的起始地址*/
	char * mmap_ptr;/*队列的起始地址，加上offset就是数据区的起始地址*/
	memset(S_pItem, 0x00, sizeof(S_pItem));/*每一个队列的处理结构*/

	/*
	 * 内存映射的文件名，编号
	 */
	char mmapname[128] = "";
	int mmapfile_num = 0;
	char mmapfile[64] = "Queue_file";
	/****

	 *共享内存区头部偏移量
	 *0代表可用(空闲)
	 *1代表有子进程在占用
	 *2代表有数据可用，防止进程提前退出

	 ****/
	int offset = sizeof(int);
	/*
	 * mmapsize = sizeof(SOCK_DATA) + offset but not thus
	 */
	int mmapsize = sizeof(S_pItem) + offset; /* int mmapsize = sizeof(SOCK_DATA) + offset;*/
	/*
	 *共享内存区的管理列表
	 *记录每个队列的起始地址
	 *子进程无法访问,无需加锁
	 */
	mmap_queue_info queue_info[ERM_MAXNUM];
	memset(&queue_info, 0x00, sizeof(queue_info));

	int mmap_data_flag = 1;

	char buff[1024] = "";
	for (mmapfile_num = 0; mmapfile_num < ERM_MAXNUM; mmapfile_num++) {
		sprintf(mmapname, "%s/%s%d", MMAPDIR, mmapfile, mmapfile_num);
		/*
		 * as the coder: we'll get ERM_MAXNUM S_pItem and each S_pItem contains ERM_MAXNUM SOCK_DATA, then
		 * we get ERM_MAXNUM * ERM_MAXNUM SOCK_DATA.
		 */
		queue_info[mmapfile_num].mmap_queue = Create_mmap(mmapname, mmapsize);
	}

	int fd[2], sock_sd[2], pid;
	/*
	 * 父子进程建立两个通信通道，一个管道，一个unix域套接字
	 * 管道用来传送Queue_info msg;套接字用来传送文件描述符
	 */
	pipe(fd);
	socketpair(AF_LOCAL, SOCK_STREAM, 0, sock_sd);

	pid = fork();
	if (pid == 0) {
		close(fd[1]);
		close(sock_sd[1]);
		/*
		 * 子进程负责从队列里取消息，然后创建线程处理消息
		 */
		process_manager(fd[0], sock_sd[0], lockfd);
		exit(0);
	}
	/*
	 * 父进程负责向队列里添加消息
	 * 1. create a socket and wait for the connection
	 * 2. accept one ....
	 * 3. put the info into the queue; the parent should guarantee that the info must be put into
	 * 	  one queue.
	 * 4. then parent should send such information to the child : the client socket, where this connection was stored
	 * 	  which included the queue address,the data block number.
	 */
	close(fd[0]);
	close(sock_sd[0]);
	int fw = fd[1];
	int sw = sock_sd[1];

	/*use enable*/
	queue_info[0].flag = 1;
	mmap_ptr = queue_info[0].mmap_queue;
	mmap_buf = (SOCKET_DATA*) (queue_info[0].mmap_queue + offset);
	memcpy(mmap_ptr, &mmap_data_flag, offset);
	/*mmap_buf ptr*/
	msg.Queue_addr = mmap_ptr;
	/*pipe ipc*/

	/*
	 write(fw,(char *)&msg,sizeof(msg));
	 write_fd(sw,(void*)&ret,sizeof(int),sd);

	 */

	nPort = 8888;

	CreateSock(&pSock, nPort);
	int frist = 1;//??????????????????

	while (1) {
		memset(buff, 0x00, sizeof(buff));
		memset(&pItem, 0x00, sizeof(pItem));
		log(LVLDEBUG,SYS_INFO,"++++++++++be accept\n");
		//cout << "+++++++++be accept" << endl;
		///////
		AcceptSock(&sd, pSock);
		log(LVLDEBUG,SYS_INFO,"==========af accept\n");
		//cout << "==================after accept " << endl;
		/*
		 * 接收消息
		 */

		fprintf(stderr, "------accept sd is : %d\n", sd);

		readlen = rtsp_read(sd, buff, 1024);

		fprintf(stderr,"------accept a connection...\n");
		fprintf(stderr,"------**********af read buf:%s , len=%d\n",buff,readlen);

		log(LVLDEBUG,SYS_INFO,"**********af read buf:%s , len=%d\n",buff,readlen);
		if (readlen <= -1) {
			log(LVLDEBUG,SYS_INFO,"interproxy read error");
			sleep(1);
			continue;
		}
		//消息封装到SOCK_DATA中
		pItem.data_len = readlen;
		memcpy(pItem.data, buff, readlen);
		/*
		 * 父子进程同时使用队列，要加锁
		 */write_lock(lockfd);
		////enQueue
		ret = PushToQueue(&pItem, mmap_buf);
		//设置队列，
		//according to	mmap_data_flag=1;memcpy(mmap_ptr, &mmap_data_flag, offset);
		//第一次,find_data_flag=1
		memcpy(&find_data_flag, mmap_ptr, offset);
		/*
		 *如果队列满，进程一定存在
		 *如果进程死亡，队列一定为空
		 */
		/*
		 * first ， in case of first time reach here,find_data_flag == 1,but we need it proceed so just add
		 * variable first just for only such function.and why we set find_data_flag=1, because we'll send the
		 * 'msg' to the child only when the current queue ran out.
		 */
		if (find_data_flag == 0 || frist) {/*进程已经死亡*/
			frist = 0;// first was first but last used here... --! for security or for sure???
			log(LVLDEBUG,SYS_INFO,"*******************************\n");
			write(fw, (char *) &msg, sizeof(msg));//send the msg to child.yes child is waiting for this...

			fprintf(stderr, "------write to fw thie msg addr is : %x \n", msg.Queue_addr);
		}
		/*插入队列*/
		//send the data block num and client sd to child
		//attention: the ret may be -1 !!
		//but here still send to child...
		write_fd(sw, (void*) &ret, sizeof(int), sd);// yes child is waiting for this too...

		fprintf(stderr, "------write to sw the sd : %d\n", sd);

		find_data_flag = 2;//data ready
		memcpy(mmap_ptr, &find_data_flag, offset);/*预防进程提前死亡*/
		write_unlock(lockfd);

		// thus we send the child -1, now we here handle the situation -1
		if (ret == -1) {/*节点内容大于100,检查节点管理列表*/
			log(LVLDEBUG,SYS_INFO,"***full***\n");
			// find one data block which was unused from all queue
			for (find_empty = 0; find_empty < ERM_MAXNUM; find_empty++) {
				if (queue_info[find_empty].flag == 0) {// ahaha got one queue which was unused! what a surprise!
					log(LVLDEBUG,SYS_INFO,"***find@@@%d\n",find_empty);
					// redo the things we do just now....
					queue_info[find_empty].flag = 1;
					mmap_ptr = queue_info[find_empty].mmap_queue;
					mmap_buf = (SOCKET_DATA*) (queue_info[find_empty].mmap_queue + offset);
					memcpy(mmap_ptr, &mmap_data_flag, offset);
					memset(&msg, 0x00, sizeof(msg));
					msg.Queue_addr = mmap_ptr;

					write_lock(lockfd);
					ret = PushToQueue(&pItem, mmap_buf);
					write(fw, (char *) &msg, sizeof(msg));
					write_fd(sw, (void*) &ret, sizeof(int), sd);
					write_unlock(lockfd);

					break;

				}

				/*节点用过每20个，回头检查一下内存区标识*/
				if (find_empty % 20 == 0) {
					for (find_tmp = 0; find_tmp < ERM_MAXNUM; find_tmp++) {
						memcpy(&find_data_flag, queue_info[find_tmp].mmap_queue, offset);
						if (find_data_flag == 0)
							queue_info[find_tmp].flag = 0;
					}
				}
				/*所有节点满，循环检查，直到空节点被发现*/
				if (find_empty >= ERM_MAXNUM - 1) {
					log(LVLDEBUG,SYS_INFO,"***all full***\n");
					sleep(1);
					find_empty = -1;
				}
			}
		}
		//the accepted connection is handled by the child
		close(sd);
	}
}
void *pthread_R6(void *arg) {
	int ret, sd;
	int i = *(int*) arg;
	//cout << "----------------" << i << endl;
	SOCKET_DATA pItem;
	memset(&pItem, 0x00, sizeof(pItem));
	pthread_mutex_lock(&p_lock);
	sd = data[i].sd;
	pItem.data_len = data[i].data_len;
	memcpy(&pItem.data, data[i].data, pItem.data_len);
	data[i].data_len = 0;
	memset(data[i].data, 0x00, sizeof(data[i].data));
	pthread_mutex_unlock(&p_lock);

	fprintf(stderr,"------********pthread read buf=%s, buflen=%d\n",pItem.data,pItem.data_len);

	log(LVLDEBUG,SYS_INFO,"********pthread read buf=%s, buflen=%d\n",pItem.data,pItem.data_len);

	ret = pthread_TP(pItem.data, pItem.data_len, sd);

	close(sd);
	pthread_mutex_lock(&p_lock);
	data[i].data_flag = 0;
	pthread_num--;
	pthread_mutex_unlock(&p_lock);
	log(LVLDEBUG,SYS_INFO,"pthread exit\n");
	pthread_exit(0);
}

void *pthread_Ermi(void *arg) {
	int ret, sd;
	int i = *(int*) arg;
	//cout << "----------------" << i << endl;
	SOCKET_DATA pItem;
	memset(&pItem, 0x00, sizeof(pItem));
	pthread_mutex_lock(&p_lock);
	sd = data[i].sd;
	pItem.data_len = data[i].data_len;
	memcpy(&pItem.data, data[i].data, pItem.data_len);
	data[i].data_len = 0;
	memset(data[i].data, 0x00, sizeof(data[i].data));
	pthread_mutex_unlock(&p_lock);

	fprintf(stderr,"------********pthread read buf=%s, buflen=%d\n",pItem.data,pItem.data_len);

	log(LVLDEBUG,SYS_INFO,"********pthread read buf=%s, buflen=%d\n",pItem.data,pItem.data_len);

	ret = pthread_Ermi_TP(pItem.data, pItem.data_len, sd);

	close(sd);
	pthread_mutex_lock(&p_lock);
	data[i].data_flag = 0;
	pthread_num--;
	pthread_mutex_unlock(&p_lock);
	log(LVLDEBUG,SYS_INFO,"pthread exit\n");
	pthread_exit(0);
}


int process_manager(int fr, int sr, int lockfd) {
	int datalen = 0, ret = 0, len = 0, pid = 0;
	int mmap_data_flag = 0;
	int find_data_flag = 1;
	int offset = sizeof(int);
	int c = 0;
	Queue_info msg;
	SOCKET_DATA pItem;
	SOCKET_DATA *mmap_buf;
	char *mmap_ptr;
	memset(&pItem, 0x00, sizeof(SOCKET_DATA));
	memset(&data, 0x00, sizeof(data));

	int i, sd;
	len = sizeof(msg);
	//pthread_t threadid;
	//pthread_attr_t attr;
	//pthread_attr_init(&attr);
	//pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	struct strrecvfd recvfd;
	Signal(SIGCHLD, child_exit);
	int pipefd[2];
	int empty_i = 0;
	memset(process_destory, 0x0, sizeof(process_destory));
	int prepid = 0;
	while (1) {

		// 线程池
	    //TPool tp(128);

		memset(&msg, 0x00, len);
		/*pipe ipc*/
		/*
		 * this process first block here to read the Queue_info msg which was send by the parent process,
		 * this pointer points to the where?
		 */
		ret = read(fr, (char *) &msg, len);

		fprintf(stderr, "------read from fr the msg addr is: %x\n", msg.Queue_addr);

		/*mmap ptr*/
		mmap_ptr = msg.Queue_addr;
		mmap_buf = (SOCKET_DATA*) (mmap_ptr + offset);
		if (ret <= 0) {
			exit(0);
			/*异常处理*/
		}
		if (process_num > ERM_MAXNUM) {
			log(LVLDEBUG,SYS_INFO,"Process num > %d\n",ERM_MAXNUM);
			/*不会产生*/
			/*异常处理*/
			//exit(0);
		}
		process_num++;
		pipe(pipefd);
		while (1) {
			pid = fork();
			if (pid < 0) {
				log(LVLDEBUG,SYS_INFO,"Fork error\n");
				sleep(1);
				continue;
			} else
				break;
		}
		if (pid) {//parent
			for (empty_i = 0; empty_i < ERM_MAXNUM; empty_i++) {
				if (process_destory[empty_i].pid == 0) {
					process_destory[empty_i].pid = pid;
					process_destory[empty_i].pid_queue_addr = mmap_ptr;
					break;
				}
			}

			close(pipefd[1]);
			/*等待子进程关闭 sr描述符*/
			ret = read(pipefd[0], NULL, 1);
			log(LVLDEBUG,SYS_INFO,"pipepipepipepipepipe\n", sd);
			sleep(100);
			close(pipefd[0]);
		} else if (!pid) {//child

			//prepid = pid;
			//if(pid != prepid)
			TPool tp(3);

			fprintf(stderr,"------child process %d runing\n",getpid());

			log(LVLDEBUG,SYS_INFO,"child process %d runing\n",getpid());
			close(pipefd[0]);
			close(fr);
			while (1) {
				if (pthread_num > ERM_MAXNUM) {/*异常，不会产生*/
					log(LVLDEBUG,SYS_INFO,"Pthread full\n");
					usleep(1000);
					continue;
				}
				memset(&pItem, 0x00, sizeof(pItem));
				find_data_flag = 1;
				if (c != -1) {
					read_fd(sr, &c, sizeof(int), &sd);

					fprintf(stderr, "------read from sr the accept sd is : %d\n", sd);
					log(LVLDEBUG,SYS_INFO,"------read from sr the accept sd is : %d\n", sd);
				}
				if (c == -1) {/*full*/
					log(LVLDEBUG,SYS_INFO,"fulllllllllllllllllllllllllllllllllllll", sd);
					write(pipefd[1], "", 1);/*可以省，直接用pipe破碎*/
					close(pipefd[1]);
					close(sr);
					if (ret != -1) {
						write_lock(lockfd);
						ret = PopFromQueue(c, &pItem, mmap_buf);//???????????????????????
						memcpy(mmap_ptr, &find_data_flag, offset);/*在自己负责的队列上做标记*/
						write_unlock(lockfd);
						ret = -1;
					}
				} else {
					write_lock(lockfd);
					ret = PopFromQueue(c, &pItem, mmap_buf);
					/*
					 *数据已读
					 */
					//cout << "ret:" << ret << endl;
					log(LVLDEBUG,SYS_INFO,"ret is %d\n", ret);
					memcpy(mmap_ptr, &find_data_flag, offset);/*在自己负责的队列上做标记*/
					write_unlock(lockfd);
				}
				if (ret != -1) {/*have data*/
					for (i = 0; i < ERM_MAXNUM; i++) {
						/*
						 *p_flag=0&&d_flag=0 :read enable
						 *p_flag=1&&d_flag=1 :pthread run
						 *p_flag=1&&d_flag=0 :pthread done
						 */
						if (data[i].pthread_flag == 0 && data[i].data_flag == 0) {
							pthread_mutex_lock(&p_lock);
							data[i].data_flag = 1;
							data[i].pthread_flag = 1;

							data[i].data_id = ret;
							data[i].sd = sd;
							data[i].data_len = pItem.data_len;
							memcpy(data[i].data, pItem.data, pItem.data_len);

							pthread_num++;
							pthread_mutex_unlock(&p_lock);
							//while (1) {
							string str(data[i].data);
						    int start = str.find("Require:");
						    int end = str.find("\r\n",start+1);
						    string req = str.substr(start,end-start);
						    req = req.substr(9,4);
						    //cout << "req is " << req << endl;
						    //cout << "start:" << start << "end:" << end << endl;
							if(req=="ngod"){
								R6Job *r6job = new R6Job(pthread_num);
								//cout << "===============================================" << req << endl;
								tp.run(r6job,(void*)&i,true);
								//cout << "===============================================" << req << endl;
							}else if(req=="ermi"){
								ErmiJob *ermijob = new ErmiJob(pthread_num);
								//cout << "+++++++++++++++++++++++++++++++++++++++++++++++" << req << endl;
								tp.run(ermijob,(void*)&i,true);
								//cout << "+++++++++++++++++++++++++++++++++++++++++++++++" << req << endl;
							}else{
								//cout << "whatwhatwhatwhatwhatwhatwhatwhatwhatwhatwhatwhat[[" << req << endl;
							}
								//ret = pthread_create(&threadid, &attr, pthread_R6, (void*) i);
								//if (ret == 0)
								//	break;
							//	sleep(1);
							//}
							break;
						}
					}
				} else {/*no data*/
					if (pthread_num == 0) {/*no pthread*/
						/*
						 *退出前检查
						 */write_lock(lockfd);
						memcpy(&find_data_flag, mmap_ptr, offset);
						if (find_data_flag == 2) {
							write_unlock(lockfd);
							continue;
						}
						/*
						 *标识为空节点
						 */
						memset(mmap_ptr, 0x00, sizeof(S_pItem) + offset);
						//memcpy(mmap_buf,&mmap_data_flag,offset);/*在自己负责的队列上做标记*/
						write_unlock(lockfd);
						close(lockfd);
						log(LVLDEBUG,SYS_INFO,"%d normal exit\n",getpid());
						exit(0);
					}
				}
				usleep(1);
				log(LVLDEBUG,SYS_INFO,"%d --------------------------\n",getppid());
			}//child while(1)
		}//child
	}//father
}
