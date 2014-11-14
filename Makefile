TARGET=sm
OBJS=sm_rtsp_s1_msg_process.o sm_rtsp_s3_msg_process.o sm_rtsp_s6_msg_process.o smain.o \
sm_communication_module.o sm_log.o sm_queue_manager.o sm_task_control_module.o sm_transaction.o \
sm_rtsp_public_function.o

#编译OCCI程序时所用到的头文件路径   
INCLUDE_PATH=\
	-I ${ORACLE_HOME}/rdbms/demo\
	-I ${ORACLE_HOME}/rdbms/public\
	-I ${ORACLE_HOME}/plsql/public\
	-I ${ORACLE_HOME}/network/public\
	-I ${ORACLE_HOME}/precomp/public\
	-I .   
#编译OCCI程序时所用到的静态链接库路径   
LIB_PATH=-L${ORACLE_HOME}/lib/\
	-L${ORACLE_HOME}/rdbms/lib/\ 
#编译OCCI程序时所用到的静态链接库   
LIBS=-lm -lnsl -lclntsh -locci  

CC=g++
%.o:%.cpp *.h 
	$(CC) -c -g $< -o $@ ${INCLUDE_PATH}

$(TARGET):$(OBJS)
	$(CC) -g $(OBJS) -o $(TARGET) -lpthread -luuid ${INCLUDE_PATH} ${LIB_PATH} ${LIBS}
	mkdir -p logfile
	mkdir -p mmapdir

clean:
	rm -rf *.o $(TARGET)
	rm -rf core.*
	rm -rf ./mmapdir
