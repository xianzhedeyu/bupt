################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../ErmiJob.o \
../R6Job.o \
../TThread.o \
../TThreadPool.o \
../erm_commnu_module.o \
../erm_db_operate_module.o \
../erm_qam_msg_monitor.o \
../erm_queuemanager.o \
../erm_rtsp_r6_msg_process.o \
../erm_rtsp_s6_msg_process.o \
../erm_task_control_module.o \
../erm_transaction.o \
../erm_vrep_d6_msg_process.o \
../ermain.o \
../ermi.o \
../ermi1.o \
../ermlog.o 

CPP_SRCS += \
../ErmiJob.cpp \
../R6Job.cpp \
../TThread.cpp \
../TThreadPool.cpp \
../erm_commnu_module.cpp \
../erm_db_operate_module.cpp \
../erm_qam_msg_monitor.cpp \
../erm_queuemanager.cpp \
../erm_rtsp_r6_msg_process.cpp \
../erm_rtsp_s6_msg_process.cpp \
../erm_task_control_module.cpp \
../erm_transaction.cpp \
../erm_vrep_d6_msg_process.cpp \
../ermain.cpp \
../ermi.cpp \
../ermi1.cpp \
../ermi_db_operate_module.cpp \
../ermi_transaction.cpp \
../ermlog.cpp 

OBJS += \
./ErmiJob.o \
./R6Job.o \
./TThread.o \
./TThreadPool.o \
./erm_commnu_module.o \
./erm_db_operate_module.o \
./erm_qam_msg_monitor.o \
./erm_queuemanager.o \
./erm_rtsp_r6_msg_process.o \
./erm_rtsp_s6_msg_process.o \
./erm_task_control_module.o \
./erm_transaction.o \
./erm_vrep_d6_msg_process.o \
./ermain.o \
./ermi.o \
./ermi1.o \
./ermi_db_operate_module.o \
./ermi_transaction.o \
./ermlog.o 

CPP_DEPS += \
./ErmiJob.d \
./R6Job.d \
./TThread.d \
./TThreadPool.d \
./erm_commnu_module.d \
./erm_db_operate_module.d \
./erm_qam_msg_monitor.d \
./erm_queuemanager.d \
./erm_rtsp_r6_msg_process.d \
./erm_rtsp_s6_msg_process.d \
./erm_task_control_module.d \
./erm_transaction.d \
./erm_vrep_d6_msg_process.d \
./ermain.d \
./ermi.d \
./ermi1.d \
./ermi_db_operate_module.d \
./ermi_transaction.d \
./ermlog.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


