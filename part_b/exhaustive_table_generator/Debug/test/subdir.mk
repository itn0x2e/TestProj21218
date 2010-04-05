################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../test/AlphabetTest.c \
../test/DictionaryTest.c \
../test/PasswordGeneratorTest.c \
../test/test.c 

OBJS += \
./test/AlphabetTest.o \
./test/DictionaryTest.o \
./test/PasswordGeneratorTest.o \
./test/test.o 

C_DEPS += \
./test/AlphabetTest.d \
./test/DictionaryTest.d \
./test/PasswordGeneratorTest.d \
./test/test.d 


# Each subdirectory must supply rules for building sources it contributes
test/%.o: ../test/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


