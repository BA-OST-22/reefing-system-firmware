################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/sensors/lsm6dsr.c \
../Core/Src/sensors/lsm6dsr_reg.c \
../Core/Src/sensors/max6675.c \
../Core/Src/sensors/ms5607.c 

OBJS += \
./Core/Src/sensors/lsm6dsr.o \
./Core/Src/sensors/lsm6dsr_reg.o \
./Core/Src/sensors/max6675.o \
./Core/Src/sensors/ms5607.o 

C_DEPS += \
./Core/Src/sensors/lsm6dsr.d \
./Core/Src/sensors/lsm6dsr_reg.d \
./Core/Src/sensors/max6675.d \
./Core/Src/sensors/ms5607.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/sensors/%.o: ../Core/Src/sensors/%.c Core/Src/sensors/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Src -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../USB/App -I../USB/Target -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-sensors

clean-Core-2f-Src-2f-sensors:
	-$(RM) ./Core/Src/sensors/lsm6dsr.d ./Core/Src/sensors/lsm6dsr.o ./Core/Src/sensors/lsm6dsr_reg.d ./Core/Src/sensors/lsm6dsr_reg.o ./Core/Src/sensors/max6675.d ./Core/Src/sensors/max6675.o ./Core/Src/sensors/ms5607.d ./Core/Src/sensors/ms5607.o

.PHONY: clean-Core-2f-Src-2f-sensors

