TARGET=ERM
OBJS=erm_commnu_module.o erm_queuemanager.o erm_task_control_module.o ermain.o ermlog.o \
erm_db_operate_module.o erm_rtsp_r6_msg_process.o erm_rtsp_s6_msg_process.o erm_transaction.o \
erm_qam_msg_monitor.o erm_vrep_d6_msg_process.o ermi.o ermi1.o TThread.o TThreadPool.o R6Job.o \
ErmiJob.o ermi_transaction.o ermi_db_operate_module.o 
CC=g++
%.o:%.cpp *.h
	$(CC) -c -g -I. $< -o $@ -lsqlite3 -L/usr/local/sqlite3/lib -I/usr/local/sqlite3/include

$(TARGET):$(OBJS)
	$(CC) -g $(OBJS) -o $(TARGET) -lpthread -lsqlite3 -L/usr/local/sqlite3/lib -I/usr/local/sqlite3/include
	mkdir -p logfile
	mkdir -p mmapdir
	mkdir -p ERMDB
clean:
	rm -f *.o
	rm -f $(TARGET)
	rm -f ./mmapdir/*

