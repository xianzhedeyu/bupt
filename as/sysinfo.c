#include "sysinfo.h"
    
static unsigned long long lastTotalUser,lastTotalUserLow,lastTotalSys,lastTotalIdle;
//static double percent2=0;
//static struct sysinfo memInfo;
void
initCpuValue(){
        FILE* file=fopen("/proc/stat", "r");
        fscanf(file,"cpu %Ld %Ld %Ld %Ld",&lastTotalUser,&lastTotalUserLow,&lastTotalSys,&lastTotalIdle);
        fclose(file);
}
/*cpupercent
 */
double 
getCurrentCpuValue(){
        double percent;
        FILE* file;
        unsigned long long totalUser,totalUserLow,totalSys,totalIdle,total;
    
        file=fopen("/proc/stat","r");
        fscanf(file,"cpu %Ld %Ld %Ld %Ld", &totalUser,&totalUserLow,&totalSys,&totalIdle);
        fclose(file);
    
        if(totalUser<lastTotalUser||totalUserLow<lastTotalUserLow||totalSys<lastTotalSys||totalIdle<lastTotalIdle){
            //溢出
            percent=-1.0;
        }
        else{
            total=(totalUser-lastTotalUser)+(totalUserLow-lastTotalUserLow)+(totalSys-lastTotalSys);
            percent=total;
            total+=(totalIdle-lastTotalIdle);
            percent/=total;
            percent*=100;
	    //percent=100-percent;//空闲CPU时间百分比
	   // percent2=(totalIdle-lastTotalIdle)/2;
	   // percent2=100-percent2;
        }

        lastTotalUser=totalUser;
        lastTotalUserLow=totalUserLow;
        lastTotalSys=totalSys;
        lastTotalIdle=totalIdle;

        return percent;
}

/*mempercent
 */
double
getCurrentMemInfo(){
	double percent;
	FILE *file;
	unsigned long long totalPhysMem;
	unsigned long long freePhysMem;
	unsigned long long sharedPhysMem;
	unsigned long long bufferPhysMem;
	file=fopen("/proc/meminfo","r");
	fscanf(file,"MemTotal: %Ld kB\n",&totalPhysMem);
	fscanf(file,"MemFree: %Ld kB\n",&freePhysMem);
	fscanf(file,"Buffers: %Ld kB\n",&bufferPhysMem);
	fscanf(file,"Cached: %Ld kB\n",&sharedPhysMem);
	percent=freePhysMem+bufferPhysMem+sharedPhysMem;
	percent/=totalPhysMem;
//	printf("%lld %lld %lld %lld\n",totalPhysMem,freePhysMem,bufferPhysMem,sharedPhysMem);
	return (1 - percent);
}

/*getloadavg获得三个数值，
 *第一个是上一分钟的平均负载，
 *第二个是最近5分钟的平均负载，
 *第三个是最近15的平均负载。
 *最好采用第三个
 *double load[3];
 *getLoadAvge(load,3)
 */
int
getLoadAvge(double load[],int n)
{
	if(getloadavg(load,3)!=-1)
	{
		return 0;
	}
	else{
		return -1;
	}
}
int
main(void)
{
	double cpupercent=0;
	double mempercent=0;
	double load[3];
//	long long totalPhysMem;
//	long long freePhysMem;
//	long long sharedPhysMem;
//	long long bufferPhysMem;
	initCpuValue();
	while(1)
	{
		sleep(20);
		cpupercent=getCurrentCpuValue();
		printf("cpu: %5.2f\n",cpupercent);
	//	printf("%5.2f %5.2f\n",cpupercent,percent2);
	//	sysinfo(&memInfo);
	//	totalPhysMem=memInfo.totalram;
	//	totalPhysMem*=memInfo.mem_unit;
	//	freePhysMem=memInfo.freeram;
	//	sharedPhysMem=memInfo.sharedram;
	//	bufferPhysMem=memInfo.bufferram;
	//	mempercent=freePhysMem+sharedPhysMem+bufferPhysMem;
		mempercent=getCurrentMemInfo();
		printf("mem: %5.2f\n",mempercent * 100);
		if(getLoadAvge(load,3)==0)
			printf("load average: %f, %f, %f\n",load[0],load[1],load[2]);
	}
	return 0;
}
