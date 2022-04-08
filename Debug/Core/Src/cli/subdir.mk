################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/cli/cli.c \
../Core/Src/cli/cli_commands.c \
../Core/Src/cli/settings.c 

OBJS += \
./Core/Src/cli/cli.o \
./Core/Src/cli/cli_commands.o \
./Core/Src/cli/settings.o 

C_DEPS += \
./Core/Src/cli/cli.d \
./Core/Src/cli/cli_commands.d \
./Core/Src/cli/settings.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/cli/%.o: ../Core/Src/cli/%.c Core/Src/cli/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Src -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../USB/App -I../USB/Target -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-cli

clean-Core-2f-Src-2f-cli:
	-$(RM) ./Core/Src/cli/cli.d ./Core/Src/cli/cli.o ./Core/Src/cli/cli_commands.d ./Core/Src/cli/cli_commands.o ./Core/Src/cli/settings.d ./Core/Src/cli/settings.o

.PHONY: clean-Core-2f-Src-2f-cli

