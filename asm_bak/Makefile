CC = gcc
CFLAGGS = -g

asm: asm.o asm_rtsp_s7_msg_process.o asm_rtsp_public_function.o
	$(CC) -o asm asm.o asm_rtsp_s7_msg_process.o

asm.o: asm.c asm_rtsp_public_function.h asm_rtsp_s7_msg_process.h
	$(CC) $(CFLAGGS) -c asm.c

asm_rtsp_s7_msg_process.o:asm_rtsp_s7_msg_process.c asm_rtsp_s7_msg_process.h asm_rtsp_public_function.h
	$(CC) $(CFLAGGS) -c asm_rtsp_s7_msg_process.c

asm_rtsp_public_function.o: asm_rtsp_public_function.cpp asm_rtsp_public_function.h
	$(CC) $(CFLAGGS) -c asm_rtsp_public_function.cpp

clean:
	-rm asm.o asm_rtsp_public_function.o asm_rtsp_s7_msg_process.o
