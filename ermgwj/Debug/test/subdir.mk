################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../test/ermi.cpp \
../test/ermlog.cpp \
../test/testgetparam.cpp \
../test/testsetup.cpp \
../test/testsetup2.cpp \
../test/testsetup3.cpp \
../test/testteardown.cpp \
../test/testteardown3.cpp 

OBJS += \
./test/ermi.o \
./test/ermlog.o \
./test/testgetparam.o \
./test/testsetup.o \
./test/testsetup2.o \
./test/testsetup3.o \
./test/testteardown.o \
./test/testteardown3.o 

CPP_DEPS += \
./test/ermi.d \
./test/ermlog.d \
./test/testgetparam.d \
./test/testsetup.d \
./test/testsetup2.d \
./test/testsetup3.d \
./test/testteardown.d \
./test/testteardown3.d 


# Each subdirectory must supply rules for building sources it contributes
test/%.o: ../test/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


