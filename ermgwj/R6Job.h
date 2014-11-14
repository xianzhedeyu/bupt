/*
 * R6Job.h
 *
 *  Created on: Jan 29, 2013
 *      Author: orion
 */

#ifndef R6JOB_H_
#define R6JOB_H_
#include "TThread.h"
#include "TThreadPool.h"
#include "erm_task_control_module.h"
#include <iostream>
using namespace std;
using namespace ThreadPool;


class R6Job: public TPool::TJob {

public:
	R6Job(int n) :
		TPool::TJob(n) {
	};
	virtual void run(void *arg){cout << "r6r6r6r6r6r6r6r6r6r6r6r6r6r6r6r6" << endl;cout << *(int*)arg << endl;pthread_R6(arg);};
	virtual ~R6Job() {
	};
};

#endif /* R6JOB_H_ */
