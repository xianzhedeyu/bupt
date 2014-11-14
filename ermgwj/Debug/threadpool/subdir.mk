################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../threadpool/TThread.o \
../threadpool/TThreadPool.o 

CPP_SRCS += \
../threadpool/TThread.cpp \
../threadpool/TThreadPool.cpp 

CC_SRCS += \
../threadpool/main.cc 

OBJS += \
./threadpool/TThread.o \
./threadpool/TThreadPool.o \
./threadpool/main.o 

CC_DEPS += \
./threadpool/main.d 

CPP_DEPS += \
./threadpool/TThread.d \
./threadpool/TThreadPool.d 


# Each subdirectory must supply rules for building sources it contributes
threadpool/%.o: ../threadpool/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

threadpool/%.o: ../threadpool/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


