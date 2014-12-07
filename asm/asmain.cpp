#include"asm_task_control_module.h"
#include"asm_queuemanager.h"
#include"public_def.h"
#include"asmain.h"
//#include"erm_qam_msg_monitor.h"
pthread_mutex_t log_lock = PTHREAD_MUTEX_INITIALIZER;
int LVLDEBUG = LVLDEBUGON;
int main() {
	int lockfd;
	int pid;
	lockfd = open(LOCKFILE, O_RDWR | O_CREAT, LOCKMODE);
	if (lockfd < 0) {
		return 1;
	}
	//DB_create();
	//DB_initializtion();
	//pid = fork();
	//if (pid < 0) {
	//	exit(1);
	//} else if (pid == 0) {
//	//	as_msg_process();
	//}
	interproxy(lockfd);
	exit(0);
}
