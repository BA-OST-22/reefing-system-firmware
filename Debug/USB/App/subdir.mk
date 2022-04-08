################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../USB/App/usb_device.c \
../USB/App/usbd_cdc_if.c \
../USB/App/usbd_desc.c 

OBJS += \
./USB/App/usb_device.o \
./USB/App/usbd_cdc_if.o \
./USB/App/usbd_desc.o 

C_DEPS += \
./USB/App/usb_device.d \
./USB/App/usbd_cdc_if.d \
./USB/App/usbd_desc.d 


# Each subdirectory must supply rules for building sources it contributes
USB/App/%.o: ../USB/App/%.c USB/App/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Src -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../USB/App -I../USB/Target -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-USB-2f-App

clean-USB-2f-App:
	-$(RM) ./USB/App/usb_device.d ./USB/App/usb_device.o ./USB/App/usbd_cdc_if.d ./USB/App/usbd_cdc_if.o ./USB/App/usbd_desc.d ./USB/App/usbd_desc.o

.PHONY: clean-USB-2f-App

