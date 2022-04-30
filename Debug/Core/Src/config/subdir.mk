################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/config/config.c \
../Core/Src/config/eeprom.c \
../Core/Src/config/globals.c 

OBJS += \
./Core/Src/config/config.o \
./Core/Src/config/eeprom.o \
./Core/Src/config/globals.o 

C_DEPS += \
./Core/Src/config/config.d \
./Core/Src/config/eeprom.d \
./Core/Src/config/globals.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/config/%.o: ../Core/Src/config/%.c Core/Src/config/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Src -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../USB/App -I../USB/Target -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/ -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -O1 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-config

clean-Core-2f-Src-2f-config:
	-$(RM) ./Core/Src/config/config.d ./Core/Src/config/config.o ./Core/Src/config/eeprom.d ./Core/Src/config/eeprom.o ./Core/Src/config/globals.d ./Core/Src/config/globals.o

.PHONY: clean-Core-2f-Src-2f-config

