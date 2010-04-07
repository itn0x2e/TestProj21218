################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../auth_file.o \
../authenticate.o \
../create_authentication.o \
../md5.o \
../misc.o \
../sha1.o \
../ui.o 

C_SRCS += \
../auth_file.c \
../authenticate.c \
../create_authentication.c 

OBJS += \
./auth_file.o \
./authenticate.o \
./create_authentication.o 

C_DEPS += \
./auth_file.d \
./authenticate.d \
./create_authentication.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


