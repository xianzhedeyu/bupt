#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <linux/kernel.h>

void initCpuValue();
double getCurrentCpuValue();
double getCurrentMemInfo();
int getLoadAvge(double load[],int n);
