################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/tasks/task_buzzer.c \
../Core/Src/tasks/task_cli.c \
../Core/Src/tasks/task_fsm.c \
../Core/Src/tasks/task_heater.c \
../Core/Src/tasks/task_sensor_read.c \
../Core/Src/tasks/task_state_est.c 

OBJS += \
./Core/Src/tasks/task_buzzer.o \
./Core/Src/tasks/task_cli.o \
./Core/Src/tasks/task_fsm.o \
./Core/Src/tasks/task_heater.o \
./Core/Src/tasks/task_sensor_read.o \
./Core/Src/tasks/task_state_est.o 

C_DEPS += \
./Core/Src/tasks/task_buzzer.d \
./Core/Src/tasks/task_cli.d \
./Core/Src/tasks/task_fsm.d \
./Core/Src/tasks/task_heater.d \
./Core/Src/tasks/task_sensor_read.d \
./Core/Src/tasks/task_state_est.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/tasks/%.o: ../Core/Src/tasks/%.c Core/Src/tasks/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Src -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../USB/App -I../USB/Target -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-tasks

clean-Core-2f-Src-2f-tasks:
	-$(RM) ./Core/Src/tasks/task_buzzer.d ./Core/Src/tasks/task_buzzer.o ./Core/Src/tasks/task_cli.d ./Core/Src/tasks/task_cli.o ./Core/Src/tasks/task_fsm.d ./Core/Src/tasks/task_fsm.o ./Core/Src/tasks/task_heater.d ./Core/Src/tasks/task_heater.o ./Core/Src/tasks/task_sensor_read.d ./Core/Src/tasks/task_sensor_read.o ./Core/Src/tasks/task_state_est.d ./Core/Src/tasks/task_state_est.o

.PHONY: clean-Core-2f-Src-2f-tasks

