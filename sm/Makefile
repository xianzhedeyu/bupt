TARGET=sm
OBJS=sm_rtsp_s1_msg_process.o sm_rtsp_s3_msg_process.o sm_rtsp_s6_msg_process.o smain.o \
sm_communication_module.o sm_log.o sm_queue_manager.o sm_task_control_module.o sm_transaction.o \
sm_rtsp_public_function.o sm_rtsp_s7_msg_process.o

LIBS=-lm -lnsl 

CC=g++
%.o:%.cpp *.h 
	$(CC) -c -g $< -o $@

$(TARGET):$(OBJS)
	$(CC) -g $(OBJS) -o $(TARGET) -lpthread -luuid -lhiredis ${LIBS}
	mkdir -p logfile
	mkdir -p mmapdir

clean:
	rm -rf *.o $(TARGET)
	rm -rf core.*
	rm -rf ./mmapdir
