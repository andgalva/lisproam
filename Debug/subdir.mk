################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../cksum.o \
../cmdline.o \
../lispd.o \
../lispd_afi.o \
../lispd_config.o \
../lispd_iface_list.o \
../lispd_iface_mgmt.o \
../lispd_input.o \
../lispd_lib.o \
../lispd_local_db.o \
../lispd_locator.o \
../lispd_log.o \
../lispd_map_cache.o \
../lispd_map_cache_db.o \
../lispd_map_notify.o \
../lispd_map_register.o \
../lispd_map_reply.o \
../lispd_map_request.o \
../lispd_mapping.o \
../lispd_nonce.o \
../lispd_output.o \
../lispd_pkt_lib.o \
../lispd_rloc_probing.o \
../lispd_smr.o \
../lispd_sockets.o \
../lispd_timers.o \
../lispd_tun.o 

C_SRCS += \
../andrea.c \
../cksum.c \
../cmdline.c \
../lispd.c \
../lispd_afi.c \
../lispd_config.c \
../lispd_iface_list.c \
../lispd_iface_mgmt.c \
../lispd_input.c \
../lispd_lib.c \
../lispd_local_db.c \
../lispd_locator.c \
../lispd_log.c \
../lispd_map_cache.c \
../lispd_map_cache_db.c \
../lispd_map_notify.c \
../lispd_map_register.c \
../lispd_map_reply.c \
../lispd_map_request.c \
../lispd_mapping.c \
../lispd_nonce.c \
../lispd_output.c \
../lispd_pkt_lib.c \
../lispd_rloc_probing.c \
../lispd_smr.c \
../lispd_sockets.c \
../lispd_timers.c \
../lispd_tun.c 

OBJS += \
./andrea.o \
./cksum.o \
./cmdline.o \
./lispd.o \
./lispd_afi.o \
./lispd_config.o \
./lispd_iface_list.o \
./lispd_iface_mgmt.o \
./lispd_input.o \
./lispd_lib.o \
./lispd_local_db.o \
./lispd_locator.o \
./lispd_log.o \
./lispd_map_cache.o \
./lispd_map_cache_db.o \
./lispd_map_notify.o \
./lispd_map_register.o \
./lispd_map_reply.o \
./lispd_map_request.o \
./lispd_mapping.o \
./lispd_nonce.o \
./lispd_output.o \
./lispd_pkt_lib.o \
./lispd_rloc_probing.o \
./lispd_smr.o \
./lispd_sockets.o \
./lispd_timers.o \
./lispd_tun.o 

C_DEPS += \
./andrea.d \
./cksum.d \
./cmdline.d \
./lispd.d \
./lispd_afi.d \
./lispd_config.d \
./lispd_iface_list.d \
./lispd_iface_mgmt.d \
./lispd_input.d \
./lispd_lib.d \
./lispd_local_db.d \
./lispd_locator.d \
./lispd_log.d \
./lispd_map_cache.d \
./lispd_map_cache_db.d \
./lispd_map_notify.d \
./lispd_map_register.d \
./lispd_map_reply.d \
./lispd_map_request.d \
./lispd_mapping.d \
./lispd_nonce.d \
./lispd_output.d \
./lispd_pkt_lib.d \
./lispd_rloc_probing.d \
./lispd_smr.d \
./lispd_sockets.d \
./lispd_timers.d \
./lispd_tun.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


