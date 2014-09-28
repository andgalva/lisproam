################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../andrea/andrea.o \
../andrea/vector.o 

C_SRCS += \
../andrea/andrea.c \
../andrea/vector.c 

OBJS += \
./andrea/andrea.o \
./andrea/vector.o 

C_DEPS += \
./andrea/andrea.d \
./andrea/vector.d 


# Each subdirectory must supply rules for building sources it contributes
andrea/%.o: ../andrea/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


