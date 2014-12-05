#include "ermlog.h"
#include<string.h>
/*
pthread_mutex_t log_lock=PTHREAD_MUTEX_INITIALIZER;
int LVLDEBUG=LVLDEBUGON;
*/
extern pthread_mutex_t log_lock;
extern int LVLDEBUG;
void ChangeAscii2String(char *pStr, char *pAscii, int len)
{
	int i,j;

	j = 2*len+1;
	memset(pStr,0,j);
	for(i=0;i<len;i++)
		sprintf(pStr,"%s%02x",pStr,(unsigned char)pAscii[i]);
}
long getfilelen(FILE *fd)
{
	long pos ,oldpos;
	oldpos=ftell(fd);
	do{
		if(fseek(fd,0,SEEK_END)==-1){
			pos=-1;
			break;
		}
		pos=ftell(fd);
		if(pos==-1){
			pos=-1;
			break;
		}
	}while(0);
	fseek(fd,oldpos,SEEK_SET);
	return pos;
}
int thelog(int lvl,int idnum,const char * func,int line,char *fmt, ...) 
{
	if( lvl==LVLDEBUGOFF)
		return 0;
	 char *s=":";
	switch(lvl){
		case LVLSYS:
			s=": SYSINFO => ";
			break;
		case LVLERR:
			s=": ERROR => ";
			break;
		case LVLDEBUGON:
			s=": DEBUG => ";
			break;
	}
	long pos;
	/*page[0] 存放LVLSYS级日志的文件个数，page[1]~page[n]存放通道日志的文件个数*/
	/*而通道号从0 开始，所以从page[idnum+1] 开始存放通道日志*/
	static int page[10]={0,0,0,0,0,0,0,0,0,0};
	/*记录前一天时间，为了重置page[]*/
	static char lastfilenametime[12]="";
	/*记录最大通道数*/
	static int maxpage=1;
	char timenowbuf[128];
	char filenametime[12];
	char logbuf[4096];
	/*通道日志的目录*/
	char logdir[128]="";
	/*LVLSYS级日志的目录*/
	char syslogdir[128]="";
	time_t tm_now;
	FILE *fd;
	if(maxpage<idnum)
		maxpage=idnum;
	tm_now=time(NULL);
	pthread_mutex_lock(&log_lock);
	strftime(timenowbuf,sizeof(timenowbuf),"%F %T",localtime(&tm_now));
	strftime(filenametime,sizeof(timenowbuf),"%F",localtime(&tm_now));
	if(strcmp(lastfilenametime,filenametime)){
		strcpy(lastfilenametime,filenametime);
		//memset(page,0,4*maxpage);
		page[0]=0;
	}

	sprintf(logbuf,"%s IN %s-%dL (%d)",timenowbuf,func,line,pthread_self());
	strcat(logbuf,s);
	if(idnum<SYS_INFO){
		char temp[]="";
		sprintf(temp,"Turnk id is %d ,help\n",idnum);
		strcat(logbuf,temp);
		idnum=SYS_INFO;
	}
	if(idnum==SYS_INFO){
		sprintf(syslogdir,"./logfile/transationinfo-p%d.log",page[0]);
		fd=fopen(syslogdir,"a+");
		if(fd==NULL){
			pthread_mutex_unlock(&log_lock);
			return 0;
		}
		pos=getfilelen(fd);
		if(pos>MAXFILELEN){
			fclose(fd);
			page[0]++;
			sprintf(syslogdir,"./logfile/transationinfo-p%d.log",page[0]);
			fd=fopen(syslogdir,"a+");
			if(fd==NULL){
				pthread_mutex_unlock(&log_lock);
				return 0;
			}
		}
	} else {
		sprintf(logdir,"./logfile/%dturnk-%s-p%d.log",idnum,filenametime,page[idnum+1]);
		fd=fopen(logdir,"a+");
		if(fd==NULL){
			pthread_mutex_unlock(&log_lock);
			return 0;
		}
                pos=getfilelen(fd);
                if(pos>MAXFILELEN){
                        fclose(fd);
                        page[idnum+1]++;
			sprintf(logdir,"./logfile/%dturnk-%s-p%d.log",idnum,filenametime,page[idnum+1]);
			fd=fopen(logdir,"a+");
			if(fd==NULL){
			pthread_mutex_unlock(&log_lock);
			return 0;
			}
             	}
	}
	va_list ap;
	va_start(ap, fmt);
	vsprintf(logbuf+strlen(logbuf),fmt,ap);
//	strcat(logbuf,"\n");
	fputs(logbuf,fd);
	fclose(fd);
	va_end(ap);
	pthread_mutex_unlock(&log_lock);
}
