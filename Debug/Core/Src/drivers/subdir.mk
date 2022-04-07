################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/drivers/adc.c \
../Core/Src/drivers/buzzer.c \
../Core/Src/drivers/dcdc.c \
../Core/Src/drivers/sleep.c 

OBJS += \
./Core/Src/drivers/adc.o \
./Core/Src/drivers/buzzer.o \
./Core/Src/drivers/dcdc.o \
./Core/Src/drivers/sleep.o 

C_DEPS += \
./Core/Src/drivers/adc.d \
./Core/Src/drivers/buzzer.d \
./Core/Src/drivers/dcdc.d \
./Core/Src/drivers/sleep.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/drivers/%.o: ../Core/Src/drivers/%.c Core/Src/drivers/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Core/Src -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../USB/App -I../USB/Target -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-drivers

clean-Core-2f-Src-2f-drivers:
	-$(RM) ./Core/Src/drivers/adc.d ./Core/Src/drivers/adc.o ./Core/Src/drivers/buzzer.d ./Core/Src/drivers/buzzer.o ./Core/Src/drivers/dcdc.d ./Core/Src/drivers/dcdc.o ./Core/Src/drivers/sleep.d ./Core/Src/drivers/sleep.o

.PHONY: clean-Core-2f-Src-2f-drivers

