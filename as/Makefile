TARGET=as
OBJS=as.o as_rtsp_r8_msg_process.o as_rtsp_public_function.o aslog.o
CC=g++
%.o:%.cpp *.h
	$(CC) -c -g -I. $< -o $@

$(TARGET):$(OBJS)
	$(CC) -g $(OBJS) -o $(TARGET) -luuid -lpthread
	mkdir -p logfile

clean:
	rm -f *.o
	rm -f $(TARGET)
