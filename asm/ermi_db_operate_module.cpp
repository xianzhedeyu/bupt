#include "ermi_db_operate_module.h"
#include"ermlog.h"
#include "time.h"
extern int LVLDEBUG;
extern int request_bw;
/*
 * sql_type: pending...;
 * 			0 : undefined; maybe DML
 * 			1 : todo; maybe DCL
 * 			2 : DDL; now complete the create table
 * 			3 : maybe DQL
 *
 */

int EDB_exect(sqlite3 *db, const char * sql, const int sql_type, const char * tablename)//----sql_type:0 DDL ;1 DML;2 create table.--- execute sql //
{
	char *eMsg;
	int ncount = 1; /*according to the code, the parameter was no used*/
	int rc;
	char tmpstr[MAX_STRING];

	/* sql_type : maybe "switch"*/
	if (sql_type == 2) {
		while (1) {
			rc = sqlite3_exec(db, sql, 0, 0, &eMsg);
			if (rc == SQLITE_OK) //建表成功
				//log(LVLDEBUG,SYS_INFO,"exect '%s' successful!\n",sql);
				return 0;
			else {
				if (strstr(eMsg, "database is locked")) {
					usleep(1000);
					ncount++;
					continue;
				} else {
					memset(tmpstr, 0x00, sizeof(tmpstr));
					sprintf(tmpstr, "table %s already exists", tablename);
					if (strstr(eMsg, tmpstr)) {

						break;
					} else {
						log(LVLDEBUG,SYS_INFO,"%s\n",eMsg);

						return -2;
					}
				}
			}
			break;
		}

	}

	if (sql_type == 1) {
		while (1) {
			rc = sqlite3_exec(db, sql, 0, 0, &eMsg);
			if (rc == SQLITE_OK) //
				//log(LVLDEBUG,SYS_INFO,"exect '%s' successful!\n",sql);
				return 0;
			else {
				if (strstr(eMsg, "database is locked")) {
					usleep(1000);
					ncount++;
					continue;
				} else {
					log(LVLDEBUG,SYS_INFO,"%s\n",eMsg);
					return -2;
				}
			}
			break;
		}

	}

	return 0;

}
/*
 * create table:
 * 				qam_info
 * 				qam_next_server
 * 				qam_udp
 * 				qam_input
 */

int EDB_create() {
	sqlite3 *db = NULL;
	int rc;
	int ncount = 0;
	char *eMsg;
	char sql_cmd[1536];
	memset(sql_cmd, 0x00, sizeof(sql_cmd));
	int result;
	char tmpstr[MAX_STRING];

	rc = sqlite3_open(DB_NAME, &db);
	if (rc != SQLITE_OK) {
		//open db failed
		free(db);
		return -1;
	}

	/*
	 * see : erm_db_operate_module.h
	 * table: qam_info
	 * 			Qam_name pk			--> Qam名称
	 * 			Qam_group_name		--> Qam组名称
	 * 			totalbw				--> 总带宽
	 * 			Frequency			--> 频率
	 * 			Modmode				--> 模式
	 * 			tsid				--> tsid
	 * 			Interleaver			--> ?
	 * 			Annex				--> ?
	 * 			Channelwidth		--> 频道带宽
	 * 			Valid				--> 是否可用
	 * 			Qam_ip				--> Qam_Ip
	 */
	sprintf(sql_cmd, "create table qam_info(Qam_name varchar(%d) PRIMARY KEY,Qam_group_name varchar(%d),totalbw INTEGER,cost INTEGER,Frequency INTEGER,Modmode varchar(%d),tsid INTEGER,Interleaver char(1),Annex char(1),Channelwidth INTEGER,Valid INTEGER,Qam_ip varchar(%d));", MAX_STRING, MAX_STRING, MAX_STRING, MAX_IP);
	memset(tmpstr, 0x00, sizeof(tmpstr));
	sprintf(tmpstr, "qam_info");
	rc = EDB_exect(db, sql_cmd, 2, tmpstr);
	log(LVLDEBUG,SYS_INFO,"rc:%d\n",rc);
	if (rc != 0) {
		sqlite3_close(db);
		return rc;
	}

	memset(sql_cmd, 0x00, sizeof(sql_cmd));
	sprintf(sql_cmd, "create table qam_next_server(Qam_name varchar(%d) , Streaming_zone_name varchar(%d), Next_server_add varchar(%d),next_server_type INTEGER);", MAX_STRING, MAX_STRING, MAX_STRING);
	memset(tmpstr, 0x00, sizeof(tmpstr));
	sprintf(tmpstr, "qam_next_server");
	rc = EDB_exect(db, sql_cmd, 2, tmpstr);

	if (rc != 0) {
		sqlite3_close(db);

		return rc;
	}

	memset(sql_cmd, 0x00, sizeof(sql_cmd));
	sprintf(sql_cmd, "create table qam_udp(Qam_name varchar(%d),udp_port INTEGER, ProgramID INTEGER, availablebw  INTEGER,udp_type INTEGER, Class_num  INTEGER ,qam_session varchar(%d),ondemand_session varchar(%d),state_time varchar(%d),use_bw INTEGER);", MAX_STRING, MAX_STRING, MAX_STRING);
	memset(tmpstr, 0x00, sizeof(tmpstr));
	sprintf(tmpstr, "qam_udp");
	rc = EDB_exect(db, sql_cmd, 2, tmpstr);

	if (rc != 0) {
		sqlite3_close(db);

		return rc;
	}

	memset(sql_cmd, 0x00, sizeof(sql_cmd));
	sprintf(sql_cmd, "create table qam_input(Qam_name varchar(%d),SubnetMask varchar(%d), Input_Host varchar(%d), PortID_1  INTEGER,PortID_2  INTEGER,PortID_3  INTEGER, MaxGroupBW  INTEGER, GroupName  varchar(%d));", MAX_STRING, MAX_STRING, MAX_STRING, MAX_STRING);
	memset(tmpstr, 0x00, sizeof(tmpstr));
	sprintf(tmpstr, "qam_input");
	rc = EDB_exect(db, sql_cmd, 2, tmpstr);

	if (rc != 0) {
		sqlite3_close(db);

		return rc;
	}

	sqlite3_close(db);

	return 0;
}

/*
 * delete the rows from the table qam_info, qam_input, qam_next_server, qam_input
 */
int EDB_initializtion() {
	sqlite3 *db = NULL;
	int rc;
	int ncount = 0;
	char *eMsg;
	char sql_cmd[1536];
	memset(sql_cmd, 0x00, sizeof(sql_cmd));
	int result;

	rc = sqlite3_open(DB_NAME, &db);
	if (rc != SQLITE_OK) { //open db failed
		return -1;
	}

	sprintf(sql_cmd, "delete from qam_info");
	rc = EDB_exect(db, sql_cmd, 1, NULL);

	if (rc != 0) {
		result = sqlite3_exec(db, "rollback transaction", 0, 0, &eMsg);
		sqlite3_close(db);
		return rc;
	}

	memset(sql_cmd, 0x00, sizeof(sql_cmd));
	sprintf(sql_cmd, "delete from qam_udp");
	rc = EDB_exect(db, sql_cmd, 1, NULL);

	if (rc != 0) {
		result = sqlite3_exec(db, "rollback transaction", 0, 0, &eMsg);
		sqlite3_close(db);
		return rc;
	}

	memset(sql_cmd, 0x00, sizeof(sql_cmd));
	sprintf(sql_cmd, "delete from qam_input");
	rc = EDB_exect(db, sql_cmd, 1, NULL);

	if (rc != 0) {
		result = sqlite3_exec(db, "rollback transaction", 0, 0, &eMsg);
		sqlite3_close(db);
		return rc;
	}

	memset(sql_cmd, 0x00, sizeof(sql_cmd));
	sprintf(sql_cmd, "delete from qam_next_server");
	rc = EDB_exect(db, sql_cmd, 1, NULL);

	if (rc != 0) {
		result = sqlite3_exec(db, "rollback transaction", 0, 0, &eMsg);
		sqlite3_close(db);
		return rc;
	}
	result = sqlite3_exec(db, "commit transaction", 0, 0, &eMsg); //提交事务
	sqlite3_close(db);

}

int EQAM_ADD(int infocheck, const eqaminfo infocontent, int nextservercheck, const eqamnextserver nextservercontent, int udpcheck, const eqamudp udpcontent, int inputcheck, const eqaminput inputcontent)
//infocheck:0,no qam_info data;1,have qam_info data   nextservercheck:0,no qam next hop server;1 have next hop server udpcheck:0,no;1 have inputcheck:0,no;1 have//
{
	sqlite3 *db = NULL;
	int rc;
	char sql_cmd[1536];
	memset(sql_cmd, 0x00, sizeof(sql_cmd));

	char *eMsg, *sql;
	int result;

	int nrow = 0, ncolumn = 0;

	//fprintf(stderr,"qam_name-%s, totalbw=%d\n",infocontent.Qam_name,infocontent.totalbw);
	//sleep(100);

	//-------------connect db------------------------	//
	rc = sqlite3_open(DB_NAME, &db);
	if (rc != SQLITE_OK) {
		//open db failed
		return -1;
	}

	log(LVLDEBUG,SYS_INFO,"qam_info qam_a:%d,cn:%d,cost:%d,fre:%d,inter:%d,mod:%s,group:%s,ip:%s,name:%s,tota:%d,tsid:%d,valid:%d\n",
			infocontent.Annex,infocontent.Channelwidth,infocontent.cost,infocontent.Frequency,infocontent.Interleaver,infocontent.Modmode
			,infocontent.Qam_group_name,infocontent.Qam_ip,infocontent.Qam_name,infocontent.totalbw,infocontent.tsid,infocontent.Valid);

	result = sqlite3_exec(db, "begin transaction", 0, 0, &eMsg); //开始一个事务


	//---------------------- insert or update qaminfo---------------------------//

	if (infocheck == 1) {
		memset(sql_cmd, 0, sizeof(sql_cmd));
		char **azResult; //存放结果
		int nrow = 0, ncolumn = 0;

		sprintf(sql_cmd, "SELECT count(*) numb FROM qam_info where qam_name='%s' ;", infocontent.Qam_name);
		sqlite3_get_table(db, sql_cmd, &azResult, &nrow, &ncolumn, &eMsg);
		log(LVLDEBUG,SYS_INFO,"select qam_info numb:%s\n",azResult[1]);
		memset(sql_cmd, 0, sizeof(sql_cmd));

		//++++++++insert qam info +++++++++++//
		if (strcmp(azResult[1], "0") == 0) {

			sprintf(sql_cmd, "INSERT INTO qam_info VALUES('%s','%s',%d,%d,%d,'%s',%d,'%d','%d',%d,%d,'%s');", infocontent.Qam_name, infocontent.Qam_group_name, infocontent.totalbw, infocontent.cost, infocontent.Frequency, infocontent.Modmode, infocontent.tsid, infocontent.Interleaver, infocontent.Annex, infocontent.Channelwidth, infocontent.Valid, infocontent.Qam_ip);
			log(LVLDEBUG,SYS_INFO,"qam info sql:%s\n",sql_cmd);
			rc = EDB_exect(db, sql_cmd, 1, NULL);

			if (rc != 0) {
				result = sqlite3_exec(db, "rollback transaction", 0, 0, &eMsg);
				sqlite3_close(db);
				return rc;
			}

		}

		//++++++++update qam info+++++++++++++//		
		if (strcmp(azResult[1], "0") == 1) //insert
		{
			char tmpstr[MAX_STRING + 20];
			sprintf(sql_cmd, "update qam_info set");

			if (infocontent.Qam_group_name != NULL) {
				memset(tmpstr, 0x00, sizeof(tmpstr));
				sprintf(tmpstr, " Qam_group_name='%s', ", infocontent.Qam_group_name);
				strcat(sql_cmd, tmpstr);
			}

			if (infocontent.Qam_ip != NULL) {
				memset(tmpstr, 0x00, sizeof(tmpstr));
				sprintf(tmpstr, " Qam_ip='%s', ", infocontent.Qam_ip);
				strcat(sql_cmd, tmpstr);
			}

			if (infocontent.Annex != '\0') {
				memset(tmpstr, 0x00, sizeof(tmpstr));
				sprintf(tmpstr, " Annex='%d', ", infocontent.Annex);
				strcat(sql_cmd, tmpstr);
			}

			if (infocontent.Channelwidth != 0) {
				memset(tmpstr, 0x00, sizeof(tmpstr));
				sprintf(tmpstr, " Channelwidth=%d, ", infocontent.Channelwidth);
				strcat(sql_cmd, tmpstr);
			}

			if (infocontent.cost != 0) {
				memset(tmpstr, 0x00, sizeof(tmpstr));
				sprintf(tmpstr, " cost=%d,", infocontent.cost);
				strcat(sql_cmd, tmpstr);
			}

			if (infocontent.Frequency != 0) {
				memset(tmpstr, 0x00, sizeof(tmpstr));
				sprintf(tmpstr, " Frequency='%d',", infocontent.Frequency);
				strcat(sql_cmd, tmpstr);
			}

			if (infocontent.Interleaver != '\0') {
				memset(tmpstr, 0x00, sizeof(tmpstr));
				sprintf(tmpstr, " Interleaver='%d',", infocontent.Interleaver);
				strcat(sql_cmd, tmpstr);
			}

			if (infocontent.Modmode != NULL) {
				memset(tmpstr, 0x00, sizeof(tmpstr));
				sprintf(tmpstr, " Modmode='%s',", infocontent.Modmode);
				strcat(sql_cmd, tmpstr);
			}

			if (infocontent.totalbw != 0) {
				memset(tmpstr, 0x00, sizeof(tmpstr));
				sprintf(tmpstr, " totalbw=%d,", infocontent.totalbw);
				strcat(sql_cmd, tmpstr);
			}

			if (infocontent.tsid != 0) {
				memset(tmpstr, 0x00, sizeof(tmpstr));
				sprintf(tmpstr, " tsid=%d,", infocontent.tsid);
				strcat(sql_cmd, tmpstr);
			}

			if (infocontent.Valid != 0) {
				memset(tmpstr, 0x00, sizeof(tmpstr));
				sprintf(tmpstr, " Valid=%d,", infocontent.Valid);
				strcat(sql_cmd, tmpstr);
			}

			memset(tmpstr, 0x00, sizeof(tmpstr));
			//log(LVLDEBUG,SYS_INFO,"length %d\n",strlen(sql_cmd));

			sql_cmd[strlen(sql_cmd) - 1] = '\0';
			sprintf(tmpstr, " where qam_name='%s';", infocontent.Qam_name);
			strcat(sql_cmd, tmpstr);

			rc = EDB_exect(db, sql_cmd, 1, NULL);

			if (rc != 0) {
				result = sqlite3_exec(db, "rollback transaction", 0, 0, &eMsg);
				sqlite3_close(db);
				return rc;
			}

		}
	}

	//------------------------------insert or update qamnexteserver---------------------------------//
	if (nextservercheck == 1) {
		log(LVLDEBUG,SYS_INFO,"next serveraddr:%s,standby_numb:%d,qam_name:%s,stream_name:%s\n",nextservercontent.Next_server_add,nextservercontent.Next_server_add_standby_numb,nextservercontent.Qam_name,nextservercontent.Streaming_zone_name);
		//++++++++insert next_hop_server primarycontent++++++++//
		if (nextservercontent.Streaming_zone_name != NULL && nextservercontent.Next_server_add != NULL) {
			memset(sql_cmd, 0x00, sizeof(sql_cmd));
			sprintf(sql_cmd, "delete from  Qam_next_server where qam_name='%s' and next_server_type=0;", nextservercontent.Qam_name);
			rc = EDB_exect(db, sql_cmd, 1, NULL);

			if (rc != 0) {
				result = sqlite3_exec(db, "rollback transaction", 0, 0, &eMsg);
				sqlite3_close(db);
				return rc;
			}

			memset(sql_cmd, 0x00, sizeof(sql_cmd));
			sprintf(sql_cmd, "INSERT INTO Qam_next_server VALUES('%s','%s','%s',0);", nextservercontent.Qam_name, nextservercontent.Streaming_zone_name, nextservercontent.Next_server_add);
			rc = EDB_exect(db, sql_cmd, 1, NULL);

			if (rc != 0) {
				result = sqlite3_exec(db, "rollback transaction", 0, 0, &eMsg);
				sqlite3_close(db);
				return rc;
			}
		}

		//++++++++++insert next_hop_server standby content++++++++//
		if (nextservercontent.Next_server_add_standby_numb > 0) {
			memset(sql_cmd, 0x00, sizeof(sql_cmd));
			sprintf(sql_cmd, "delete from  Qam_next_server where qam_name='%s' and next_server_type=0;", nextservercontent.Qam_name);

			rc = EDB_exect(db, sql_cmd, 1, NULL);

			if (rc != 0) {
				result = sqlite3_exec(db, "rollback transaction", 0, 0, &eMsg);
				sqlite3_close(db);
				return rc;
			}

			int i;
			for (i = 1; i <= nextservercontent.Next_server_add_standby_numb; i++) {
				memset(sql_cmd, 0x00, sizeof(sql_cmd));

				sprintf(sql_cmd, "INSERT INTO Qam_next_server VALUES('%s',NULL,'%s',1);", nextservercontent.Qam_name, nextservercontent.Next_server_add_standby[i]);
				rc = EDB_exect(db, sql_cmd, 1, NULL);

				if (rc != 0) {
					result = sqlite3_exec(db, "rollback transaction", 0, 0, &eMsg);
					sqlite3_close(db);
					return rc;
				}
			}

		}
	}

	//---------------------------------------insert or update qamudp----------------------------------//
	if (udpcheck == 1) {
		//log(LVLDEBUG,SYS_INFO,"",udpcontent);
		if (udpcontent.udp_numb > 0) {
			memset(sql_cmd, 0x00, sizeof(sql_cmd));
			sprintf(sql_cmd, "delete from  Qam_udp where qam_name='%s' and Class_num=1 and udp_type=0;", udpcontent.Qam_name);
			rc = EDB_exect(db, sql_cmd, 1, NULL);

			if (rc != 0) {
				result = sqlite3_exec(db, "rollback transaction", 0, 0, &eMsg);
				sqlite3_close(db);
				return rc;
			}
			int i;
			for (i = 0; i < udpcontent.udp_numb; i++) {
				memset(sql_cmd, 0x00, sizeof(sql_cmd));
				sprintf(sql_cmd, "INSERT INTO Qam_udp VALUES('%s',%d,%d,null,0,1,null,null,null,null);", udpcontent.Qam_name, udpcontent.Udpport_ProgramID[i][0], udpcontent.Udpport_ProgramID[i][1]);
				rc = EDB_exect(db, sql_cmd, 1, NULL);
				//			log(LVLDEBUG,SYS_INFO,"select qam_name:%s i:%d udp:%d\n",udpcontent.Qam_name,i,udpcontent.Udpport_ProgramID[i][0]);
				if (rc != 0) {
					result = sqlite3_exec(db, "rollback transaction", 0, 0, &eMsg);
					sqlite3_close(db);
					return rc;
				}
			}

		}

		if (udpcontent.availablebw > 0) {
			memset(sql_cmd, 0x00, sizeof(sql_cmd));
			sprintf(sql_cmd, "delete from  Qam_udp where qam_name='%s' and Class_num=0;", udpcontent.Qam_name);
			rc = EDB_exect(db, sql_cmd, 1, NULL);

			if (rc != 0) {
				result = sqlite3_exec(db, "rollback transaction", 0, 0, &eMsg);
				sqlite3_close(db);
				return rc;
			}
			memset(sql_cmd, 0x00, sizeof(sql_cmd));
			sprintf(sql_cmd, "INSERT INTO Qam_udp VALUES('%s',NULL,NULL,%d,NULL,0,NULL,NULL,NULL,NULL);", udpcontent.Qam_name, udpcontent.availablebw);
			rc = EDB_exect(db, sql_cmd, 1, NULL);

			if (rc != 0) {
				result = sqlite3_exec(db, "rollback transaction", 0, 0, &eMsg);
				sqlite3_close(db);
				return rc;
			}
		}

	}

	//---------------------------------------insert or update qaminput--------------------------------//

	if (inputcheck != 0) {
		if (inputcontent.input_numb > 0) {
			memset(sql_cmd, 0x00, sizeof(sql_cmd));
			sprintf(sql_cmd, "delete from  Qam_input where qam_name='%s' ;", inputcontent.Qam_name);

			rc = EDB_exect(db, sql_cmd, 1, NULL);

			if (rc != 0) {
				result = sqlite3_exec(db, "rollback transaction", 0, 0, &eMsg);
				sqlite3_close(db);
				return rc;
			}

			int i;
			for (i = 0; i < inputcontent.input_numb; i++) {

				memset(sql_cmd, 0x00, sizeof(sql_cmd));
				log(LVLDEBUG,SYS_INFO,"input num:%d, subnetmask:%s\n",i,inputcontent.SubnetMask[i]);
				sprintf(sql_cmd, "INSERT INTO Qam_input VALUES('%s','%s','%s',%d,%d,%d,%d,'%s');", inputcontent.Qam_name, inputcontent.SubnetMask[i], inputcontent.Input_Host[i], inputcontent.PortID[i][0], inputcontent.PortID[i][1], inputcontent.PortID[i][2], inputcontent.MaxGroupBW[i], inputcontent.GroupName[i]);
				rc = EDB_exect(db, sql_cmd, 1, NULL);

				if (rc != 0) {
					result = sqlite3_exec(db, "rollback transaction", 0, 0, &eMsg);
					sqlite3_close(db);
					return rc;
				}
			}
		}
	}

	result = sqlite3_exec(db, "commit transaction", 0, 0, &eMsg); //提交事务
	sqlite3_close(db);
	return 0;
}

int EQAM_SETUP_DOWN(const eqamsdinfo &qma_sd) {
	sqlite3 *db = NULL;
	int rc;
	int ncount = 0;
	char *eMsg;
	char sql_cmd[1536];
	int result;
	int tmpstate;
	char tmpstr[MAX_STRING * 3];

	rc = sqlite3_open(DB_NAME, &db);

	memset(tmpstr, 0x00, MAX_STRING * 3);

	if (rc != SQLITE_OK) {
		//open db failed
		return -1;
	}
	result = sqlite3_exec(db, "begin transaction", 0, 0, &eMsg); //开始一个事务

	if (qma_sd.udp_state == 4) {
		tmpstate = 0;
		sprintf(tmpstr, ",Qam_session=null,State_time=null,use_bw=null ");
	} else {
		tmpstate = qma_sd.udp_state;
		snprintf(tmpstr, sizeof(tmpstr), ",Qam_session='%s',State_time='%ld' ,use_bw=%d ", qma_sd.qam_session, time(NULL), qma_sd.use_bw);
	}

	memset(sql_cmd, 0x00, sizeof(sql_cmd));
	sprintf(sql_cmd, "update Qam_udp set udp_type=%d %s where qam_name='%s' and Class_num=1;", tmpstate, tmpstr, qma_sd.qam_name);
	rc = EDB_exect(db, sql_cmd, 1, NULL);

	log(LVLDEBUG,SYS_INFO,"setupsql:%s \n",sql_cmd);
	if (rc != 0) {
		result = sqlite3_exec(db, "rollback transaction", 0, 0, &eMsg);
		sqlite3_close(db);
		return rc;
	}

	if (qma_sd.udp_state == 2) {
		memset(sql_cmd, 0x00, sizeof(sql_cmd));
		sprintf(sql_cmd, "update Qam_udp set availablebw=availablebw-%d where qam_name='%s' and Class_num=0 ;", qma_sd.use_bw, qma_sd.qam_name);
		rc = EDB_exect(db, sql_cmd, 1, NULL);

		if (rc != 0) {
			result = sqlite3_exec(db, "rollback transaction", 0, 0, &eMsg);
			sqlite3_close(db);
			return rc;
		}
	}

	if (qma_sd.udp_state == 4) {
		memset(sql_cmd, 0x00, sizeof(sql_cmd));
		sprintf(sql_cmd, "update Qam_udp set availablebw=availablebw+%d where qam_name='%s' and Class_num=0 ;", qma_sd.use_bw, qma_sd.qam_name);
		rc = EDB_exect(db, sql_cmd, 1, NULL);

		if (rc != 0) {
			result = sqlite3_exec(db, "rollback transaction", 0, 0, &eMsg);
			sqlite3_close(db);
			return rc;
		}
	}
	result = sqlite3_exec(db, "commit transaction", 0, 0, &eMsg); //提交事务
	sqlite3_close(db);
	return 0;

}

/*
 * 从数据库中查询 qam_numb 个 SM 请求的 QAM 的信息，包括：
 * 1、qam_info 表：
 * 				qam_ip（QAM的地址）,
 * 				Frequency（QAM的频率）,
 * 				Modmode（调制方式？）,
 * 				Qam_group_name（QAM属于的QAM组）
 * 2、qam_next_server 表：
 * 				next_server_add（下一跳的服务器的地址）
 * 3、qam_input 表：
 * 				Input_Host,
 * 				GroupName
 * 4、qam_udp 表：（UDP端口和节目号的映射）
 * 				udp_port（UDP 端口）,
 * 				programid （节目号）
 * 5、qam_udp 表：
 * 				availablebw（可用总带宽）
 * */
int EQAM_SELECT(eqamselectinfo *qam_info, int qam_numb) {
	eqamselectinfo tmpselect[qam_numb];
	memset(&tmpselect, 0x00, sizeof(eqamselectinfo) * qam_numb);

	//将 QAM 的信息 复制给了 临时的 tmpselect
	memcpy(&tmpselect, qam_info, sizeof(eqamselectinfo) * qam_numb);

	sqlite3 *db = NULL;
	int rc;
	int ncount = 0;
	char *eMsg;
	char sql_cmd[1536];
	int result;

	rc = sqlite3_open(DB_NAME, &db);
	if (rc != SQLITE_OK) {
		//open db failed
		fprintf(stderr, "------open db error\n");
		sqlite3_close(db);
		return -1;
	}
	fprintf(stderr, "------open db success\n");
	int i = 0;
	for (i = 0; i < qam_numb; i++) {
		char **azResult; //存放结果
		char *tmpstr;
		int nrow = 0, ncolumn = 0;

		//---select qam info 
		memset(sql_cmd, 0x00, sizeof(sql_cmd));
		/*qam_info:
		 * Qam_name
		 * Qam_group_name
		 * totalbw
		 * cost
		 * Frequency
		 * Modmode
		 * tsid
		 * Interleaver
		 * Annex
		 * Channelwidth
		 * Valid
		 * Qam_ip
		 */
		fprintf(stderr, "QAMNAME:%s\n", tmpselect[i].qam_name);
		sprintf(sql_cmd, "select qam_ip,Frequency,Modmode,Qam_group_name from qam_info where qam_name='%s' ;", tmpselect[i].qam_name);
		sqlite3_get_table(db, sql_cmd, &azResult, &nrow, &ncolumn, &eMsg);
		fprintf(stderr, "qam_info->nrow:%d\n", nrow);
		if (nrow > 0) {
			memcpy(tmpselect[i].qam_add, azResult[4], sizeof(tmpselect[i].qam_add));
			tmpselect[i].Frequency = atoi(azResult[5]);
			memcpy(tmpselect[i].Modmode, azResult[6], sizeof(tmpselect[i].Modmode));
			memcpy(tmpselect[i].qam_group, azResult[7], sizeof(tmpselect[i].qam_group));
			log(LVLDEBUG,SYS_INFO,"select qam_name:%s modmode:%s qam_group:%s qam_add:%s,Frequency:%s\n",tmpselect[i].qam_name,azResult[6],azResult[7],azResult[4],azResult[5]);
			fprintf(stderr, "------select qam_name:%s modmode:%s qam_group:%s qam_add:%s,Frequency:%s\n", tmpselect[i].qam_name, azResult[6], azResult[7], azResult[4], azResult[5]);
		}

		//---select qam next hop server
		nrow = 0;
		ncolumn = 0;
		memset(sql_cmd, 0x00, sizeof(sql_cmd));
		/*qam_next_server:
		 * Qam_name
		 * Streaming_zone_name
		 * Next_server_add
		 * next_server_type
		 * */
		sprintf(sql_cmd, "select next_server_add from qam_next_server where qam_name='%s'  and next_server_type=0;", tmpselect[i].qam_name);
		sqlite3_get_table(db, sql_cmd, &azResult, &nrow, &ncolumn, &eMsg);
		fprintf(stderr, "qam_next_server->nrow:%d\n", nrow);
		if (nrow > 0) {
			char *falg = ":";
			tmpstr = strtok(azResult[1], falg);
			fprintf(stderr, "tmpstr:%s\n", tmpstr);
			memcpy(tmpselect[i].next_add, tmpstr, strlen(tmpstr) + 1);
			tmpselect[i].next_port = atoi(strtok(NULL, falg));
			fprintf(stderr, "------next_add:%s,next_port:%d\n", tmpselect[i].next_add, tmpselect[i].next_port);
		}

		//---select qam input 
		nrow = 0;
		ncolumn = 0;
		memset(sql_cmd, 0x00, sizeof(sql_cmd));
		/*qam_input:
		 * Qam_name
		 * SubnetMask
		 * Input_Host
		 * PortID_1
		 * PortID_2
		 * PortID_3
		 * MaxGroupBW
		 * GroupName
		 * */
		sprintf(sql_cmd, "select Input_Host,GroupName from qam_input where qam_name='%s' ;", tmpselect[i].qam_name);
		sqlite3_get_table(db, sql_cmd, &azResult, &nrow, &ncolumn, &eMsg);
		fprintf(stderr, "qam_input->nrow:%d\n", nrow);
		if (nrow > 0) {
			memcpy(tmpselect[i].input_add, azResult[2], sizeof(tmpselect[i].input_add));
			memcpy(tmpselect[i].input_group, azResult[3], sizeof(tmpselect[i].input_group));
			fprintf(stderr, "------inputadd:%s,input_group:%s\n", tmpselect[i].input_add, tmpselect[i].input_group);
		}

		//---select qam input 
		nrow = 0;
		ncolumn = 0;
		memset(sql_cmd, 0x00, sizeof(sql_cmd));
		/*qam_udp:
		 * Qam_name
		 * |udp_port
		 * |ProgramID
		 * |availablebw
		 * |udp_type
		 * |Class_num
		 * |qam_session
		 * |ondemand_session
		 * |state_time
		 * |use_bw
		 * */
		sprintf(sql_cmd, "select udp_port,programid from qam_udp where qam_name='%s' and Class_num=1 and Udp_type=0 ;", tmpselect[i].qam_name);
		sqlite3_get_table(db, sql_cmd, &azResult, &nrow, &ncolumn, &eMsg);
		fprintf(stderr, "qam_udp1->nrow:%d\n", nrow);
		if (nrow > 0) {
			int count = 1;
			tmpselect[i].udp_num = nrow;
			/* 00 01
			 * 10 11
			 * ...
			 * [count-1][0] [count-1][1]
			 *//*端口和节目号的映射？*/
			for (count = 1; count < nrow + 1; count++) {
				tmpselect[i].udp_program[count - 1][0] = atoi(azResult[count * ncolumn]);
				tmpselect[i].udp_program[count - 1][1] = atoi(azResult[count * ncolumn + 1]);
			}
			log(LVLDEBUG,SYS_INFO,"udp:%d program:%d \n",tmpselect[i].udp_program[0][0],tmpselect[i].udp_program[0][1]);
			fprintf(stderr, "udp:%d program:%d \n", tmpselect[i].udp_program[0][0], tmpselect[i].udp_program[0][1]);
		}

		nrow = 0;
		ncolumn = 0;
		memset(sql_cmd, 0x00, sizeof(sql_cmd));
		sprintf(sql_cmd, "select availablebw from qam_udp where qam_name='%s'  and Class_num=0 ;", tmpselect[i].qam_name);
		sqlite3_get_table(db, sql_cmd, &azResult, &nrow, &ncolumn, &eMsg);
		fprintf(stderr, "qam_udp2->nrow:%d\n", nrow);
		if (nrow > 0) {
			tmpselect[i].available_bw = atoi(azResult[1]);
			fprintf(stderr, "available_bw:%d\n", tmpselect[i].available_bw);
		}
	}

	//memcpy(qam_info, &tmpselect, sizeof(qamselectinfo) * qam_numb);
	log(LVLDEBUG,SYS_INFO,"request_bw:%d\n",request_bw);
	qamsort(tmpselect, 0, qam_numb);

	int j = 0;
	eqamselectinfo res;
	for (j = 0; j < qam_numb; j++) {
		log(LVLDEBUG,SYS_INFO,"－－－－－－%d\n",tmpselect[j].available_bw);
		log(LVLDEBUG,SYS_INFO,"－－－－－－%d\n",request_bw);
		if (tmpselect[j].available_bw > request_bw) {

			res = tmpselect[j];
			break;
		}
	}
	if (j == qam_numb)
		return j;
	memcpy(qam_info, &res, sizeof(eqamselectinfo));
	log(LVLDEBUG,SYS_INFO,"request_bw:%d,available_bw:%d\n",request_bw,res.available_bw);
	sqlite3_close(db);
	return 0;
}

int EQAM_DOWN_SELECT(eqamselectinfo_down *qs) {

	eqamselectinfo_down tmpselect;
	memset(&tmpselect, 0x00, sizeof(eqamselectinfo_down));
	memcpy(&tmpselect, qs, sizeof(eqamselectinfo_down));

	sqlite3 *db = NULL;
	int rc;
	int ncount = 0;
	char *eMsg;
	char **azResult; //存放结果
	char sql_cmd[1536];
	int result;
	char tmpstr[MAX_STRING];
	char *tmp;
	int nrow = 0, ncolumn = 0;
	rc = sqlite3_open(DB_NAME, &db);
	if (rc != SQLITE_OK) {
		//open db failed
		return -1;
	}
	//log(LVLDEBUG,SYS_INFO,"select session:%s \n",tmpselect.ondemand_session);
	memset(sql_cmd, 0x00, sizeof(sql_cmd));
	memset(tmpstr, 0x00, sizeof(tmpstr));

	/* qam_udp:
	 * 存放的已连接的 SS－> QAM 的信息*/
	sprintf(sql_cmd, "select count(*) from qam_udp where qam_session='%s' ;", tmpselect.qam_session);
	sqlite3_get_table(db, sql_cmd, &azResult, &nrow, &ncolumn, &eMsg);
	int tmpnum;
	tmpnum = atoi(azResult[1]);
	if (tmpnum > 0) {
		memset(sql_cmd, 0x00, sizeof(sql_cmd));
		memset(tmpstr, 0x00, sizeof(tmpstr));
		sprintf(sql_cmd, "select qam_name,qam_session,use_bw from qam_udp where qam_session='%s' ;", tmpselect.qam_session);
		sqlite3_get_table(db, sql_cmd, &azResult, &nrow, &ncolumn, &eMsg);
		memcpy(tmpstr, azResult[3], sizeof(tmpstr));
		memcpy(tmpselect.qam_name, azResult[3], sizeof(tmpselect.qam_name));
		memcpy(tmpselect.qam_session, azResult[4], sizeof(tmpselect.qam_session));
		tmpselect.use_bw = atoi(azResult[5]);

		memset(sql_cmd, 0x00, sizeof(sql_cmd));
		sprintf(sql_cmd, "select Next_server_add from qam_next_server where qam_name='%s' ;", tmpstr);
		sqlite3_get_table(db, sql_cmd, &azResult, &nrow, &ncolumn, &eMsg);
		char *falg = ":";
		tmp = strtok(azResult[1], falg);
		memcpy(tmpselect.next_add, tmp, sizeof(tmpselect.next_add));
		tmpselect.next_port = atoi(strtok(NULL, falg));
	} else {
		return -10;
	}

	memcpy(qs, &tmpselect, sizeof(eqamselectinfo_down));

	return 0;
}

int EQAM_SESSION_SELECT(EGETPARAM_MSG getparam, string &res) {
	string tmphost = getparam.rtsp_url;
	string host = tmphost.substr(0, tmphost.find("/"));
	cout << "host: " << host << endl;
	string param = getparam.param;
	cout << "param: " << param << endl;
	sqlite3 *db = NULL;
	int rc;
	int ncount = 0;
	char *eMsg;
	char **azResult; //存放结果
	char sql_cmd[1536];
	int result;
	char tmpstr[MAX_STRING];
	char *tmp;
	int nrow = 0, ncolumn = 0;
	rc = sqlite3_open(DB_NAME, &db);
	if (rc != SQLITE_OK) {
		//open db failed
		return -1;
	}
	//log(LVLDEBUG,SYS_INFO,"select session:%s \n",tmpselect.ondemand_session);
	memset(sql_cmd, 0x00, sizeof(sql_cmd));
	memset(tmpstr, 0x00, sizeof(tmpstr));


	if (param == "session_list") {
		/* qam_udp:
			 * 存放的已连接的 SS－> QAM 的信息*/
			sprintf(sql_cmd, "select qam_name from qam_input where input_host='%s' ;", host.c_str());
			cout << "sql : " << sql_cmd << endl;
			sqlite3_get_table(db, sql_cmd, &azResult, &nrow, &ncolumn, &eMsg);
			int tmpnum;
			cout << "row:" << nrow << endl;
			cout << "col:" << ncolumn << endl;
			//tmpnum = atoi(azResult[1]);
			//cout << "rows: " << tmpnum << endl;
			if (nrow > 0) {
				string qam_name = string(azResult[1]);
				memset(sql_cmd, 0x00, sizeof(sql_cmd));
				memset(tmpstr, 0x00, sizeof(tmpstr));
				sprintf(sql_cmd, "select qam_session from qam_udp where qam_name='%s' group by qam_session ;", qam_name.c_str());
				cout << "sql:" << sql_cmd << endl;
				sqlite3_get_table(db, sql_cmd, &azResult, &nrow, &ncolumn, &eMsg);
				if (nrow > 0){
					cout << "---col: " << ncolumn << endl;
					cout << "---row: " << nrow << endl;
					for(int i=1;i<=nrow;i++){
						for(int j=0;j<ncolumn;j++){
							//cout << azResult[j] << " " << azResult[i*ncolumn+j] << endl;
							res += azResult[i*ncolumn + j];
						}
						if (i != nrow) res += " ";
					}
				}else{
					return -10;
				}
			} else {
				return -10;
			}

	}
	return 0;
}
void qamsort(eqamselectinfo s[], int l, int r) //使用快排，将所有EQAM按带宽排序
{
	int i, j, x;
	if (l < r) {
		i = l;
		j = r;
		x = s[i].available_bw;
		while (i < j) {
			while (i < j && s[j].available_bw > x)
				j--;
			if (i < j)
				s[i++] = s[j];
			while (i < j && s[i].available_bw < x)
				i++;
			if (i < j)
				s[j--] = s[i];
		}
		s[i].available_bw = x;
		qamsort(s, l, i - 1);
		qamsort(s, i + 1, r);
	}
}
