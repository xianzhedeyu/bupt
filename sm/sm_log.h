#ifndef _SM_LOG_H
#define _SM_LOG_H
#include <stdio.h>
#include<time.h>
#include <stdarg.h>
#include<pthread.h>
/*--------------------------------------------------------*/
/* 版本:v1.0

 * 作者:王传华

 * 创建时间:2008-08-21

 *修改记录:

 *程序说明:日志模块，负责记录日志
 * 		日志文件命名：
 *		1.系统信息：Systeminfo-p0.log
 *		(如果文件超长则p0为p1~pn)
 *		2.通道信息：1turnk-2008-08-11-p0.log
 *		(1turnk为1通道，2turnk为2通道，依次类推）

 ----------------------------------------------------------*/

#define LVLSYS 0
#define LVLERR 1
#define LVLDEBUGOFF 3 /*调试日志关闭开关*/
#define LVLDEBUGON 2  /*调试日志开启开关*/
#define MAXFILELEN (2150000000UL) /*一份日志最大长度~=2GB，单位byte*/
#define SYS_INFO -1 /*系统日志的打印标志*/
#define sm_log(x,y,...) thelog(x,y,__FILE__,__LINE__,__VA_ARGS__)
long getfilelen(FILE *fd); 
/* lvl 日志级别 分：LVLSYS ,LVLDEBUG,LVLERR
 * idnum 通道号,0 ：不按通道打印日志.当级别为LVLSYS时 通常为idnum为0
 */
int thelog(int lvl,int idnum,const char *,int line,char *fmt, ...); 
void ChangeAscii2String(char *pStr, char *pAscii, int len);
#endif
