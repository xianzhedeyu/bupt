#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/time.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include <poll.h>
#include <sys/socket.h>
#include"erm_qam_msg_monitor.h"
#include"erm_db_operate_module.h"
#include"public_def.h"
#include"erm_commnu_module.h"
#include <sys/resource.h>
#include"ermlog.h"
//#####################
#include"TThread.h"
#include"TThreadPool.h"
//####################

extern int LVLDEBUG;
stQAM_CONNECT_INFO g_QAM_CONNECT_INFO_T[100];
stQAM_FIRST_UPDATA_INFO g_stQAM_FIRST_UPDATA_INFO_T[100];

int setnonblocking(int sockfd) {
	if (fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0) | O_NONBLOCK) == -1) {
		return -1;
	}
	return 0;
}

int ChangeToTen(const char *s) {
	int i;
	int n = 0;
	for (i = 0; s[i] != '\0'; i++) {
		if (s[i] >= '0' && s[i] <= '9')
			n = n * 16 + s[i] - '0';
		if (s[i] >= 'a' && s[i] <= 'f')
			n = n * 16 + s[i] - 'a' + 10;
		if (s[i] >= 'A' && s[i] <= 'F')
			n = n * 16 + s[i] - 'A' + 10;
	}
	return n;
}

int qam_info_decode(unsigned char *d, unsigned char *s, int nDatalen) {
	unsigned char *pSupdataInfo = NULL;
	unsigned char szDupdataInfo[4096] = "";
	unsigned char ascbuf[8192];
	unsigned char ascbuf1[8];
	unsigned char ascbuf2[4096];
	int nlen = 0;
	char szLen[3] = "";
	char packagelen1[5] = "";
	int npacklen = 0;

	//	fprintf(stderr,"333333len=%d\n", nDatalen);
	//ChangeAscii2String((char*)ascbuf,(char*)s,nDatalen);
	//fprintf(stderr,"!!!!!!![%s]  共%d个字节的数据\n",ascbuf,nDatalen);

	pSupdataInfo = s;
	memcpy(szLen, pSupdataInfo, 2);

	//////////////
	memset(ascbuf1, 0x00, 8);
	ChangeAscii2String((char*) ascbuf1, (char*) szLen, 2);
	memset(packagelen1, 0x00, sizeof(packagelen1));
	memcpy(packagelen1, ascbuf1, 4);
	nlen = ChangeToTen(packagelen1);

	///////
	//nlen = atoi(szLen);
	//fprintf(stderr,"######nlen=%d, szlen=%s\n",nlen, packagelen1);
	//ChangeAscii2String((char*)ascbuf1,(char*)pSupdataInfo,nlen);
	//fprintf(stderr,"********[%s]  共%d个字节的数据\n",ascbuf1,nlen);
	memcpy(d, pSupdataInfo, nlen);
	//d=szDupdataInfo;
	//	memcpy(d, szDupdataInfo, nlen);
	//ChangeAscii2String((char*)ascbuf2,(char*)d,nlen);
	//fprintf(stderr,"@@@@@@@@@[%s]  共%d个字节的数据\n",ascbuf2,nlen);	
	return nlen;
}

void *handle_message(void *arg) {
	unsigned char buf[MAXBUF * 8 + 1];
	unsigned char ascbuf[MAXBUF * 4 + 1];

	unsigned char packagelen[3] = "";

	char packagelen1[5] = "";
	unsigned char ascbuf1[8] = "";
	int npacklen = 0;
	int nTmpPackLen = 0;
	int nMaxPackLen = 0;

	int len = 0;
	int nMsgType;
	int nTSN = (int) arg;
	int nAcceptSock = -1;
	fd_set rset;
	fd_set tem_rset;
	int maxfd = 0;
	int nRet = -1;
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 10000;
	unsigned char sztembuff[12000] = "";
	int nUpdataLen = 0;

	OPEN M_OPEN;
	UPDATE M_UPDATE;
	struct pollfd clientfd[100];
	int nmax = 0;
	int nready = 0;
	int n;
	int nSudportunm = 0;
	int nDudportunm = 0;
	int nUdpos = 0;
	int nUCount, pCount, dCount;
	// QAM 自身的信息
	qaminfo qaminfocontent;

	// QAM 下一跳服务器信息
	qamnextserver nextservercontent;

	// QAM udp 信息
	qamudp udpcontent;

	// QAM 输入端口信息
	qaminput inputcontent;

	int nSendlen = 0;
	char KeepAliveMessage[3];
	PackageKeepalive(KeepAliveMessage);
	/* 开始处理每个新连接上的数据收发 */

	// 初始化上传数据结构
	for (int c = 0; c < 100; c++) {
		g_stQAM_FIRST_UPDATA_INFO_T[c].cost = 0;
		g_stQAM_FIRST_UPDATA_INFO_T[c].nflag = 0;
		g_stQAM_FIRST_UPDATA_INFO_T[c].nsd = 0;
		g_stQAM_FIRST_UPDATA_INFO_T[c].ServiceStatus = 0;
		g_stQAM_FIRST_UPDATA_INFO_T[c].EdgeInputNum = 0;
	}
	memset(g_stQAM_FIRST_UPDATA_INFO_T, 0x00, sizeof(g_stQAM_FIRST_UPDATA_INFO_T));

	// 初始化 客户接口 为 未用状态
	for (n = 0; n < 100; n++) {
		clientfd[n].fd = -1;
	}
	fprintf(stderr, "%d handle_message pthread start......\n", nTSN);
	int pollno = 0;
	int i = 0;

	while (1) {
		nTSN = (int) arg;
		//fprintf(stderr,"ddddddTSN=%d\n",nTSN);
		for (i = 0; i < 100; i++) {
			//fprintf(stderr, "g_QAM_CONNECT_INFO_T[%d].nsock[%d]=%d\n", nTSN,i,g_QAM_CONNECT_INFO_T[nTSN].nsock[i]);

			// 从当前线程维护的 客户端 中选择未处理的
			if (g_QAM_CONNECT_INFO_T[nTSN].nsock[i] == 0) {
				usleep(100);
				continue;
			}
			fprintf(stderr, "^^^^^^^^^^^^^^^^^^^^^%d  %d\n", g_QAM_CONNECT_INFO_T[nTSN].nsock[i], i);
			// 找到
			for (pollno = 0; pollno < 100; pollno++) {
				if ((clientfd[pollno].fd == -1)) {
					clientfd[pollno].fd = g_QAM_CONNECT_INFO_T[nTSN].nsock[i];
					clientfd[pollno].events = POLLIN;
					pollno++;
					g_QAM_CONNECT_INFO_T[nTSN].nsock[i] = 0;
					g_QAM_CONNECT_INFO_T[nTSN].nflag = 0;
					g_QAM_CONNECT_INFO_T[nTSN].nNUM++;
					break;
				}
			}
		}
		nmax = g_QAM_CONNECT_INFO_T[nTSN].nNUM;
		nready = poll(clientfd, nmax + 1, 100);
		if ((nready == 0) || (nready == -1)) {
			//fprintf(stderr,"poll no socket sd\n");
			continue;
		}
		////////////////////////////////////////////////////////////////
		/*
		 * 以上代码的作用，每个线程维护自己的100个 stQAM_CONNECT_INFO 与 100个 pollfd
		 * 主线程创建子线程
		 * 1）子线程启动后就会检查自己的 stQAM_CONNECT_INFO里的nsock是否等于零（0：没有
		 *    数据；1：有数据），只要检查到一个没有数据就会等待100毫秒，然后继续检查。
		 * 2）如果找到了一个则检查 pollfd 中有没有空闲的地方，
		 * 		i、如果有，就存入 pollfd；
		 * 		ii、如果没有则返回 1）
		 * 3）如果检查完（或者stQAM_CONNECT_INFO或者pollfd）都没有找到，则继续执行
		 * 4）正常和不正常的情况都会走到这里poll，这里代码不处理异常情况而攒到了poll里，暂时
		 * 	  没有发现错误，我不太喜欢。正常情况下poll会返回大于零的值然后继续执行，异常情况下poll会在
		 * 	  100毫秒内返回小于等于零的值，这里会continue。
		 */
		////////////////////////////////////////////////////////////////
		for (n = 0; n < nmax; n++) {
			// poll返回大于零，检查套接口收到的时间的类型
			// 不是 POLLIN
			if (!clientfd[n].revents & POLLIN) {
				// 是 POLLERR，则关闭这个套接口，重置收到事件的套接口
				if (clientfd[n].revents & POLLERR) {
					fprintf(stderr, "%d queue's %d fd POLLERR", n, clientfd[n].fd);
					close(clientfd[n].fd);
					clientfd[pollno].fd = -1;
				}
				// 不是 POLLIN、POLLERR，继续检查
				continue;
			}
			//fprintf(stderr," !!!!n=%d sd=%d\n", n,clientfd[n].fd);

			// 是 POLLERR 或者 套接口没用
			/*
			 * 这里的 POLLERR 检查没有必要？，因为会在上一个if处理掉
			 */
			if ((clientfd[n].revents & POLLERR) || clientfd[n].fd == -1) {
				fprintf(stderr, "%d queue's %d fd POLLERR", n, clientfd[n].fd);
				close(clientfd[n].fd);
				clientfd[pollno].fd = -1;
				continue;
			}
			//fprintf(stderr," @@@@@@@n=%d sd=%d\n", n,clientfd[n].fd);
			// ？？？？？？
			// buf的定义：unsigned char buf[MAXBUF * 8 + 1];
			bzero(buf, MAXBUF + 1);

			/* 接收客户端的消息 */
			/*	int nRecvLen=0;
			 unsigned char szRecvbuf[4096+1];
			 while(1)
			 {
			 nRecvLen = 0;
			 memset(szRecvbuf,0x00,sizeof(szRecvbuf));
			 nRecvLen = recv(clientfd[n].fd, szRecvbuf, MAXBUF, 0);
			 memcpy(buf+nMaxPackLen,szRecvbuf,nRecvLen);
			 nMaxPackLen += nRecvLen;

			 if(nRecvLen<4096)
			 break;
			 }*/
			socklen_t nl = 0;
			struct sockaddr_in clientaddr1;
			char cl_ip[20] = "";
			memset(&clientaddr1, 0, sizeof(clientaddr1));

			nMaxPackLen = recv(clientfd[n].fd, buf, MAXBUF * 4, 0);

			// 获得对端的 IP 地址
			getpeername(clientfd[n].fd, (struct sockaddr*) &clientaddr1, &nl);
			// 转换为本地格式
			inet_ntop(AF_INET, &(clientaddr1.sin_addr), cl_ip, sizeof(clientaddr1));

			fprintf(stderr, "RecvLen=%d,socked=%d,IP=%s,nl=%d\n", nMaxPackLen, clientfd[n].fd, cl_ip, nl);

			for(int tt=0;tt<nMaxPackLen;tt++)
				//cout << "-----------" << int(buf[tt])<<".";
				printf("...%x...",buf[tt]);
			cout << endl;

			if (nMaxPackLen <= 0) {
				fprintf(stderr, "消息接收失败！错误代码是%d，错误信息是'%s' %d\n", errno, strerror(errno), nMaxPackLen);
				perror("RECV");
				close(clientfd[pollno].fd);
				clientfd[pollno].fd = -1;
				///////
				g_QAM_CONNECT_INFO_T[nTSN].nNUM--;
				///////
				continue;
			}

			nUpdataLen = 0;
			nTmpPackLen = 0;
			len = nMaxPackLen;//接收到的消息的长度
			while (len > 0) {

				memset(sztembuff, 0x00, sizeof(sztembuff));
				memset(packagelen, 0x0, sizeof(packagelen));

				// 返回消息的长度
				nUpdataLen = qam_info_decode(sztembuff, buf + nTmpPackLen, len);

				nTmpPackLen = nTmpPackLen + nUpdataLen;
				len = nMaxPackLen - nTmpPackLen;

				// 转成 16 进制
				memset(ascbuf, 0x00, sizeof(ascbuf));
				ChangeAscii2String((char*) ascbuf, (char*) sztembuff, nUpdataLen);
				// 如果消息类型不是
				/*
				 * 01:OPEN
				 * 02:UPDATE
				 * 03:NOTIFICATION
				 * 04:KEEPALIVE
				 * 中的一种
				 */
				if (sztembuff[2] != 0x01 && sztembuff[2] != 0x02 && sztembuff[2] != 0x03 && sztembuff[2] != 0x04)
					break;

				fprintf(stderr, "BBBBB[%s][packlen=%d, nTmpPackLen=%d] 共%d个字节的数据\n", ascbuf, len, nTmpPackLen, nUpdataLen);
				//调用解包接口
				//len=0;
				//sleep(1);
				switch (sztembuff[2]) {
				case VREP_ID_D6_OPEN: {
					memset(&M_OPEN, 0x00, sizeof(M_OPEN));
					if (ParseOPEN(sztembuff, M_OPEN))
						InspectOPEN(M_OPEN);//Streaming Zone，Component Name是强制参数
					//将M_OPEN写入数据库
					//open信息暂时不需要入库
					M_OPEN.Len = 38;
					M_OPEN.type = '1';
					M_OPEN.version = 2;
					M_OPEN.holdtime = 240;
					//M_OPEN.ID=new byte();
					M_OPEN.ID[0] = 0x7B;//192
					M_OPEN.ID[1] = 0x77;//168
					M_OPEN.ID[2] = 0xFD;//10
					M_OPEN.ID[3] = 0xDF;//208
					M_OPEN.parametersLen = 21;
					M_OPEN.ParameterNum = 2;
					//M_OPEN.parameters=new Parameters();
					M_OPEN.parameters[0].ParameterType = 2;
					M_OPEN.parameters[0].SubParametersLen = 6;
					//M_OPEN.parameters[0].ParameterValue=new byte();
					//M_OPEN.parameters[0].ParameterValue=(byte*)"BBN.SZ";
					memcpy(M_OPEN.parameters[0].ParameterValue, "BBN.SZ", sizeof(M_OPEN.parameters[0].ParameterValue));
					M_OPEN.parameters[1].ParameterType = 3;
					M_OPEN.parameters[1].SubParametersLen = 7;
					//M_OPEN.parameters[1].ParameterValue=new byte();
					//M_OPEN.parameters[1].ParameterValue=(byte*)"BBN.ERM";
					memcpy(M_OPEN.parameters[1].ParameterValue, "BBN.ERM", sizeof(M_OPEN.parameters[1].ParameterValue));

					char OpenMessage[M_OPEN.Len];
					PackageOpen(M_OPEN, OpenMessage);
					nSendlen = rtsp_write(clientfd[n].fd, OpenMessage, M_OPEN.Len);
					fprintf(stderr, "nSendlen %d\n", nSendlen);
					if (nSendlen < 0) {
						//错误信息处理
					}
				}
					break;
				case VREP_ID_D6_UPDATE: {
					/*	int nSudportunm = 0;
					 int nDudportunm = 0;
					 int nUdpos = 0;
					 qaminfo qaminfocontent;
					 qamnextserver nextservercontent;
					 qamudp udpcontent;
					 qaminput inputcontent;*/

					memset(&M_UPDATE, 0x00, sizeof(M_UPDATE));

					memset(&qaminfocontent, 0x00, sizeof(qaminfocontent));

					memset(&nextservercontent, 0x00, sizeof(nextservercontent));

					memset(&udpcontent, 0x00, sizeof(udpcontent));

					memset(&inputcontent, 0x00, sizeof(inputcontent));

					nSudportunm = 0;
					nDudportunm = 0;

					// 解析 UPDATE 消息
					if (ParseUPDATE(sztembuff, M_UPDATE))
						InspectUPDATE(M_UPDATE);
					// 填充 qaminfocontent 结构
					/*
					 * 1) QAM的名字
					 * 2) QAM的组名
					 * 3) QAM的总带宽
					 * 4) QAM的开销
					 * 5) QAM的频率
					 * 6) QAM的调制方式
					 * 7) QAM的TSID
					 * 8) QAM的Interleaver
					 * 9) QAM的Annex
					 * 10) QAM的频道带宽
					 * 11) QAM的IP？
					 */
					memcpy(qaminfocontent.Qam_name, M_UPDATE.QAMName[0], sizeof(qaminfocontent.Qam_name));
					if (!strcmp(qaminfocontent.Qam_name, "")) {
						// fprintf(stderr,"11111Qam_name=%s, M_UPDATE.QAMName=%s\n",qaminfocontent.Qam_name, M_UPDATE.QAMName[0]);

						// 从g_stQAM_FIRST_UPDATA_INFO_T中查找未使用的一个块
						/* 将 开销，客户套接字，服务状态，输入端口的数量，输入端口的信息
						 * 添入 结构中。干什么用的呢？？？
						 */
						for (int nQamInfoCount = 0; nQamInfoCount < 100; nQamInfoCount++) {
							if (g_stQAM_FIRST_UPDATA_INFO_T[nQamInfoCount].nflag == 0) {
								//fprintf(stderr,"cost=%d\n",M_UPDATE.cost);
								g_stQAM_FIRST_UPDATA_INFO_T[nQamInfoCount].cost = M_UPDATE.cost;
								g_stQAM_FIRST_UPDATA_INFO_T[nQamInfoCount].nflag = 1;
								g_stQAM_FIRST_UPDATA_INFO_T[nQamInfoCount].nsd = clientfd[n].fd;
								g_stQAM_FIRST_UPDATA_INFO_T[nQamInfoCount].ServiceStatus = M_UPDATE.ServiceStatus;
								g_stQAM_FIRST_UPDATA_INFO_T[nQamInfoCount].EdgeInputNum = M_UPDATE.EdgeInputNum;
								memcpy(g_stQAM_FIRST_UPDATA_INFO_T[nQamInfoCount].EdgeInput, M_UPDATE.EdgeInput, sizeof(g_stQAM_FIRST_UPDATA_INFO_T[nQamInfoCount].EdgeInput));
								break;
							}
						}
						//break;
					}

					memcpy(qaminfocontent.Qam_group_name, M_UPDATE.ReachableRoutes[0].Address, sizeof(qaminfocontent.Qam_group_name));
					qaminfocontent.totalbw = M_UPDATE.totalbw;
					qaminfocontent.cost = M_UPDATE.cost;
					qaminfocontent.Frequency = M_UPDATE.QAMP.Frequency;
					memcpy(qaminfocontent.Modmode, M_UPDATE.QAMP.Modmode, sizeof(qaminfocontent.Modmode));
					qaminfocontent.tsid = M_UPDATE.QAMP.TSID;
					qaminfocontent.Interleaver = (short) M_UPDATE.QAMP.Interleaver;
					qaminfocontent.Annex = (short) M_UPDATE.QAMP.Annex;
					qaminfocontent.Channelwidth = M_UPDATE.QAMP.Channelwidth;
					//qaminfocontent.Valid = M_UPDATE
					//test
					memcpy(qaminfocontent.Qam_ip, M_UPDATE.EdgeInput[0].Host, sizeof(qaminfocontent.Qam_ip));
					fprintf(stderr, "M_UPDATE.QAMP.Interleaver=%d, M_UPDATE.QAMP.Annex=%d\n ", qaminfocontent.Interleaver, qaminfocontent.Annex);
					//fprintf(stderr,"Qam_name=%s, M_UPDATE.QAMName=%s\n",qaminfocontent.Qam_name, M_UPDATE.QAMName[0]);
					//fprintf(stderr,"M_UPDATE.totalbw=%d\n", M_UPDATE.totalbw);


					/*
					 * 填充 nextservercontent 结构
					 * 1) QAM 名字
					 * 2) 流域的名字
					 * 3) 下一跳地址
					 * 4) 备用下一跳服务个数
					 * 5) 备用下一跳服务器
					 */
					memcpy(nextservercontent.Qam_name, M_UPDATE.QAMName[0], sizeof(nextservercontent.Qam_name));
					memcpy(nextservercontent.Streaming_zone_name, M_UPDATE.NextHopServer.ZoneName, sizeof(nextservercontent.Streaming_zone_name));
					memcpy(nextservercontent.Next_server_add, M_UPDATE.NextHopServer.Address, sizeof(nextservercontent.Next_server_add));
					nextservercontent.Next_server_add_standby_numb = M_UPDATE.Alternates.NumAlternates; //fprintf(stderr,"Next_server_add_standby_numb=%d\n", nextservercontent.Next_server_add_standby_numb);
					memcpy(nextservercontent.Next_server_add_standby, M_UPDATE.Alternates.server[0], sizeof(nextservercontent.Next_server_add_standby));

					/*
					 * 填充 udpcontent 结构
					 * 1) QAM 名字
					 * 2) QAM 静态、动态端口的映射 port －> ProgramId
					 * 3) QAM 可用带宽
					 */

					memcpy(udpcontent.Qam_name, M_UPDATE.QAMName[0], sizeof(udpcontent.Qam_name));
					// 静态端口数
					nSudportunm = M_UPDATE.SPortsNum;
					// 动态端口数
					for (pCount = 0; pCount < M_UPDATE.DPortsNum; pCount++) {
						nDudportunm += M_UPDATE.DPorts[pCount].Count;
					}

					// 总共的端口数＝静＋动
					udpcontent.udp_numb = nSudportunm + nDudportunm;
					fprintf(stderr, "udp_numb=%d nSudportunm=%d nDudportunm=%d Conut=%d, DPortsNum=%d\n", udpcontent.udp_numb, nSudportunm, nDudportunm, M_UPDATE.DPorts[0].Count, M_UPDATE.DPortsNum);

					for (nUCount = 0; nUCount < nSudportunm; nUCount++) {
						udpcontent.Udpport_ProgramID[nUCount][0] = M_UPDATE.SPorts[nUCount].UDPPort;
						udpcontent.Udpport_ProgramID[nUCount][1] = M_UPDATE.SPorts[nUCount].ProgramID;
						nUdpos++;
					}

					int nSubportunm = 0;
					for (dCount = 0; dCount < M_UPDATE.DPortsNum; dCount++) {
						for (nSubportunm = 0; nSubportunm < M_UPDATE.DPorts[dCount].Count; nSubportunm++) {
							udpcontent.Udpport_ProgramID[nSubportunm][0] = M_UPDATE.DPorts[dCount].StartingPort;
							udpcontent.Udpport_ProgramID[nSubportunm][1] = M_UPDATE.DPorts[dCount].StartingPID;
							M_UPDATE.DPorts[dCount].StartingPort++;
							M_UPDATE.DPorts[dCount].StartingPID++;
							nUdpos++;
						}
					}

					udpcontent.availablebw = M_UPDATE.availablebw;

					/*
					 * 从 g_stQAM_UPDATA_INFO_T 取出添加的数据放进 inputcontent 结构
					 * 之后释放 g_stQAM_UPDATA_INFO_T 的块
					 */

					memcpy(inputcontent.Qam_name, M_UPDATE.QAMName[0], sizeof(inputcontent.Qam_name));

					for (int k = 0; k < 100; k++) {
						// fprintf(stderr,"g_stQAM_FIRST_UPDATA_INFO_T[%d].nsd=%d,clientfd[%d].fd=%d\n",k,g_stQAM_FIRST_UPDATA_INFO_T[k].nsd, n,clientfd[n].fd);
						if (g_stQAM_FIRST_UPDATA_INFO_T[k].nsd == clientfd[n].fd) {
							qaminfocontent.cost = g_stQAM_FIRST_UPDATA_INFO_T[k].cost;
							qaminfocontent.Valid = g_stQAM_FIRST_UPDATA_INFO_T[k].ServiceStatus;
							inputcontent.input_numb = g_stQAM_FIRST_UPDATA_INFO_T[k].EdgeInputNum;
							for (int num = 0; num < inputcontent.input_numb; num++) {
								memcpy(inputcontent.SubnetMask[num], g_stQAM_FIRST_UPDATA_INFO_T[k].EdgeInput[num].SubnetMask, sizeof(inputcontent.SubnetMask[num]));
								fprintf(stderr, "subnetmask=%s k=%d\n", g_stQAM_FIRST_UPDATA_INFO_T[k].EdgeInput[num].SubnetMask, k);
								memcpy(inputcontent.Input_Host[num], g_stQAM_FIRST_UPDATA_INFO_T[k].EdgeInput[num].Host, sizeof(inputcontent.Input_Host[num]));
								inputcontent.PortID[num][0] = g_stQAM_FIRST_UPDATA_INFO_T[k].EdgeInput[num].portID.portnumber;
								inputcontent.PortID[num][1] = g_stQAM_FIRST_UPDATA_INFO_T[k].EdgeInput[num].portID.slotnumber;
								inputcontent.PortID[num][2] = g_stQAM_FIRST_UPDATA_INFO_T[k].EdgeInput[num].portID.subinterface;
								inputcontent.MaxGroupBW[num] = g_stQAM_FIRST_UPDATA_INFO_T[k].EdgeInput[num].MaxGroupBW;
								memcpy(inputcontent.GroupName[num], g_stQAM_FIRST_UPDATA_INFO_T[k].EdgeInput[num].GroupName, sizeof(inputcontent.GroupName[num]));
							}

							g_stQAM_FIRST_UPDATA_INFO_T[k].cost = 0;
							g_stQAM_FIRST_UPDATA_INFO_T[k].nflag = 0;
							g_stQAM_FIRST_UPDATA_INFO_T[k].nsd = 0;
							g_stQAM_FIRST_UPDATA_INFO_T[k].ServiceStatus = 0;
							g_stQAM_FIRST_UPDATA_INFO_T[k].EdgeInputNum = 0;
							memset(g_stQAM_FIRST_UPDATA_INFO_T[k].EdgeInput, 0x00, sizeof(g_stQAM_FIRST_UPDATA_INFO_T[k].EdgeInput));
							break;
						}
					}
					/*for(int p=0;p<256;p++)
					 {
					 fprintf(stderr,"Port num=%d\n",udpcontent.Udpport_ProgramID[p][0]);
					 }*/

					fprintf(stderr, "2222222qam_name=%s totalbw=%d, valid=%d, cost=%d, availablebw=%d\n", qaminfocontent.Qam_name, qaminfocontent.totalbw, qaminfocontent.Valid, qaminfocontent.cost, udpcontent.availablebw);

					QAM_ADD(1, qaminfocontent, 1, nextservercontent, 1, udpcontent, 1, inputcontent);

					nSendlen = rtsp_write(clientfd[n].fd, KeepAliveMessage, 3);

					fprintf(stderr, "UPDATA KEEPALIVE nSendlen %d\n", nSendlen);
				}
					break;
				case VREP_ID_D6_NOTIFICATION: {
					cout << "MessageType is: NOTIFICATION" << endl;
					ParseNOTIFICATION(sztembuff);
					//上报错误
				}
					break;
				case VREP_ID_D6_KEEPALIVE: {
					//发送KEEPALIVE消息

					nSendlen = rtsp_write(clientfd[n].fd, KeepAliveMessage, 3);
					cout << "MessageType is: KEEPALIVE" << endl;
					fprintf(stderr, "KEEPALIVE nSendlen %d\n", nSendlen);
				}
					break;
				default:
					//TODO:错误处理
					cout << "MessageType ERROR!" << endl;
					break;

				}

			}
			/*	else {
			 fprintf(stderr,"消息接收失败！错误代码是%d，错误信息是'%s' %d\n", errno, strerror(errno),len);
			 perror("RECV");
			 close(clientfd[pollno].fd);
			 clientfd[pollno].fd = -1;
			 g_QAM_CONNECT_INFO_T[nTSN].nNUM --;
			 }*/
		}

	}
	fprintf(stderr, "!!!!!!!!!!\n");
	/* 处理每个新连接上的数据收发结束 */
}

int init_qam_info() {

	memset(g_QAM_CONNECT_INFO_T, 0x0, sizeof(g_QAM_CONNECT_INFO_T));

	int k;
	for (k = 0; k < 100; k++) {
		g_QAM_CONNECT_INFO_T[k].nTID = 1;
	}

	return 0;
}
/*
 int insert_qam_info(int nTSN, int nAcceptSock,int pos)
 {
 int i;
 g_QAM_CONNECT_INFO_T[nTSN].nsock[pos]= nAcceptSock;
 g_QAM_CONNECT_INFO_T[nTSN].nNUM++;
 return i;
 g_QAM_CONNECT_INFO_T[nTSN].flag=1;
 return -1;
 }
 */
/*
 * search the g_QAM_CONNECT_INFO_T[100] to find if there is any thread unused, if got one then to check if the socket which managed by this
 * thread does free.
 * 0  - got one
 * 1  - if we got one thread, but the thread does't has socket to mark it.
 * -1 - do not got one, there isn't thread left
 */
int find_qam_info(int nTSN, int nAcceptSock) {
	int i, k;
	for (k = 0; k <= nTSN; k++) {
		if (g_QAM_CONNECT_INFO_T[k].nflag == 0) { /*is there any thread unused*/
			for (i = 0; i < 100; i++) {
				if (g_QAM_CONNECT_INFO_T[k].nsock[i] == 0) { /*is there any socket unused in the thread*/
					g_QAM_CONNECT_INFO_T[k].nsock[i] = nAcceptSock;
					g_QAM_CONNECT_INFO_T[k].nNUM++;
					break; /*if got one jump out*/
				}
			}
			if (i < 100) /*if got one */
				return 0;
			else {
				g_QAM_CONNECT_INFO_T[k].nflag = 1;
				return 1;
			}
		}
	}
	return -1;
}

//void * Thread_d6_msg_accept_dist( void *arg)
void qam_msg_process() {
	int listener, new_fd, kdpfd, nfds, n, ret, curfds;
	socklen_t len;
	struct sockaddr_in my_addr, their_addr;
	unsigned int nPort, lisnum;
	//struct epoll_event ev;
	//struct epoll_event events[MAXEPOLLSIZE];
	struct rlimit rt;
	pthread_t nthreadID;
	//----------
	nPort = 9999;
	//----------
	int nPos = -1;
	int nListenSock;
	int nAcceptSock;
	int nTSN = 0;

	init_qam_info();
////测试 先注释掉////////////////////////////////////////////////
	/* 设置每个进程允许打开的最大文件数 */
//	rt.rlim_max = rt.rlim_cur = MAXEPOLLSIZE;
//	if (setrlimit(RLIMIT_NOFILE, &rt) == -1) {
//		perror("setrlimit");
//		exit(1);
//	} else
//		fprintf(stderr, "设置系统资源参数成功！\n");
///////////////////////////////////////////////////

	if (CreateSock(&nListenSock, nPort) < 0) {
		//log(LVLERR,SYS_INFO,"Create socket error!");
		fprintf(stderr, "Create socket error!");
		exit(0);
	}
	/*
	 * create a new thread to handle D6 interface messages
	 */
	//TPool tp(3);
	pthread_create(&nthreadID, NULL, handle_message, (void*) nTSN);
	for (;;) {
		if (AcceptSock(&nAcceptSock, nListenSock) == 0) {

			fprintf(stderr, "@@@@@@@@@@@@@@\n");
			/* < to be check >
			 * why we should call find_qam_info? because when we do AcceptSock, there maybe a message arrived and the former thread(we just created)
			 * will handle it and modify the info g_QAM_CONNECT_INFO_T,so we here call find_qam_info.
			 * </ to be check >
			 */
			nPos = find_qam_info(nTSN, nAcceptSock);
			if ((nPos == 1)) { //找到一个可用线程，但是此线程管理的端口没有可用的；找到后此线程不可用了，并且创建新线程来处理请求
				fprintf(stderr, "*****Create New pthread******");
				pthread_create(&nthreadID, NULL, handle_message, (void*) nTSN);
				nTSN++;
				find_qam_info(nTSN, nAcceptSock);
			} else if (nPos < 0) {
				fprintf(stderr, "***** Queue Full****** close sd");
				close(nAcceptSock);
			}
			/*	int len;
			 char ascbuf[2048];
			 char buf[1024];
			 ///////////

			 OPEN M_OPEN;
			 M_OPEN.Len=38;
			 M_OPEN.type='1';
			 M_OPEN.version=2;
			 M_OPEN.holdtime=240;
			 M_OPEN.ID[0]=0xC0;//192
			 M_OPEN.ID[1]=0xA8;//168
			 M_OPEN.ID[2]=0x0A;//10
			 M_OPEN.ID[3]=0xD0;//208
			 M_OPEN.parametersLen=21;
			 M_OPEN.ParameterNum=2;
			 M_OPEN.parameters[0].ParameterType=2;
			 M_OPEN.parameters[0].SubParametersLen=6;
			 M_OPEN.parameters[0].ParameterValue[0]='B';
			 M_OPEN.parameters[0].ParameterValue[0]='B';
			 M_OPEN.parameters[0].ParameterValue[0]='N';
			 M_OPEN.parameters[0].ParameterValue[0]='.';
			 M_OPEN.parameters[0].ParameterValue[0]='S';
			 M_OPEN.parameters[0].ParameterValue[0]='Z';
			 M_OPEN.parameters[1].ParameterType=3;
			 M_OPEN.parameters[1].SubParametersLen=7;
			 M_OPEN.parameters[1].ParameterValue[0]='B';
			 M_OPEN.parameters[1].ParameterValue[0]='B';
			 M_OPEN.parameters[1].ParameterValue[0]='N';
			 M_OPEN.parameters[1].ParameterValue[0]='.';
			 M_OPEN.parameters[1].ParameterValue[0]='E';
			 M_OPEN.parameters[1].ParameterValue[0]='R';
			 M_OPEN.parameters[1].ParameterValue[0]='M';

			 char OpenMessage[M_OPEN.Len];
			 PackageOpen(M_OPEN,OpenMessage);
			 char KeepAliveMessage[3];
			 PackageKeepalive(KeepAliveMessage);
			 ///////
			 len=rtsp_read(nAcceptSock,buf,2048);
			 rtsp_write(nAcceptSock,OpenMessage,M_OPEN.Len);

			 ChangeAscii2String((char*)ascbuf,(char*)buf,len);
			 fprintf(stderr,"[%s]  共%d个字节的数据\n",ascbuf,len);
			 while(1)
			 {
			 len=rtsp_read(nAcceptSock,buf,2048);
			 rtsp_write(nAcceptSock,KeepAliveMessage,3);
			 ChangeAscii2String((char*)ascbuf,(char*)buf,len);
			 fprintf(stderr,"[%s]  共%d个字节的数据\n",ascbuf,len);
			 }
			 exit(1);*/
			//调用解包接口
		} else {
			fprintf(stderr, "*************8\n");
			//log(LVLERR,SYS_INFO,"Accept socket error!");
			exit(0);
		}

	}

}

