TARGET=asm
OBJS=asmain.o asm_communication_module.o asm_queuemanager.o asm_rtsp_public_function.o asm_rtsp_r8_msg_process.o asm_rtsp_s7_msg_process.o asm_task_control_module.o asm_transaction.o asmlog.o
CC=g++
%.o:%.cpp *.h
	$(CC) -c -g -I. $< -o $@ `mysql_config --cflags --libs`

$(TARGET):$(OBJS)
	$(CC) -g $(OBJS) -o $(TARGET) -lpthread -luuid -lhiredis -lm -lnsl `mysql_config --cflags --libs`
	mkdir -p logfile
	mkdir -p mmapdir
clean:
	rm -f *.o
	rm -f $(TARGET)
	rm -f ./mmapdir/*

