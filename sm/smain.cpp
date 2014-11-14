#include"sm_task_control_module.h"
#include"sm_queue_manager.h"
#include"public_def.h"
#include "sm_log.h"
#include"smain.h"
//#include"sm_qam_msg_monitor.h"
//#include "erm_db_operate_module.h"
pthread_mutex_t log_lock=PTHREAD_MUTEX_INITIALIZER;
int LVLDEBUG=LVLDEBUGON;
int main()
{
    int lockfd;
    int pid;
    lockfd=open(LOCKFILE,O_RDWR|O_CREAT,LOCKMODE);
    if (lockfd<0) {
        return 1;
    }
    interproxy(lockfd);
    exit(0);
}
