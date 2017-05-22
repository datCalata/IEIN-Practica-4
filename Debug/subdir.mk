################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../dron_control.c \
../fsm.c \
../main.c \
../msg_list.c 

OBJS += \
./dron_control.o \
./fsm.o \
./main.o \
./msg_list.o 

C_DEPS += \
./dron_control.d \
./fsm.d \
./main.d \
./msg_list.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cygwin C Compiler'
	gcc -I../ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


