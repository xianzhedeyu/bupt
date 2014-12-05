/*
 * R6Job.h
 *
 *  Created on: Jan 29, 2013
 *      Author: orion
 */

#ifndef ERMIJOB_H_
#define ERMIJOB_H_
#include "TThread.h"
#include "TThreadPool.h"
#include "erm_task_control_module.h"
#include <iostream>
#include"ermlog.h"
extern int LVLDEBUG;
using namespace std;
using namespace ThreadPool;


class ErmiJob: public TPool::TJob {

public:
	ErmiJob(int n) :
		TPool::TJob(n) {
	};
	virtual void run(void *arg){cout << "ErmiErmiErmiErmiErmiErmi" << endl;/*cout << *(int*)arg << endl;*/pthread_Ermi(arg);};
	virtual ~ErmiJob() {
	};
};

#endif /* ERMIJOB_H_ */
