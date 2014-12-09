#include <stdlib.h>
#include <stdio.h>
int
main(void)
{
	double load[3];
	if(getloadavg(load,3)!=-1)
	{
		printf("load average: %f,%f,%f\n",load[0],load[1],load[2]);
	}
	return 0;
}
