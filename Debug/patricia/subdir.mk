################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../patricia/patricia.o 

C_SRCS += \
../patricia/patricia.c 

OBJS += \
./patricia/patricia.o 

C_DEPS += \
./patricia/patricia.d 


# Each subdirectory must supply rules for building sources it contributes
patricia/%.o: ../patricia/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


