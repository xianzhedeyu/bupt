################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../tmp/l.c \
../tmp/w.c 

OBJS += \
./tmp/l.o \
./tmp/w.o 

C_DEPS += \
./tmp/l.d \
./tmp/w.d 


# Each subdirectory must supply rules for building sources it contributes
tmp/%.o: ../tmp/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


