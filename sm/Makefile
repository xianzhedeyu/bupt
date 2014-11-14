CC = g++
CFLAGGS = -g

sm: sm.o sm_rtsp_s7_msg_process.o sm_rtsp_public_function.o
	$(CC) -o sm sm.o sm_rtsp_public_function.o sm_rtsp_s7_msg_process.o -luuid

sm_rtsp_public_function.o: sm_rtsp_public_function.cpp sm_rtsp_public_function.h
	$(CC) $(CFLAGGS) -c sm_rtsp_public_function.cpp

sm_rtsp_s7_msg_process.o:sm_rtsp_s7_msg_process.c sm_rtsp_s7_msg_process.h sm_rtsp_public_function.h
	$(CC) $(CFLAGGS) -c sm_rtsp_s7_msg_process.c sm_rtsp_public_function.cpp

sm.o: sm.c sm_rtsp_public_function.h sm_rtsp_s7_msg_process.h
	$(CC) $(CFLAGGS) -c sm.c 

clean:
	-rm sm.o sm_rtsp_public_function.o sm_rtsp_s7_msg_process.o
