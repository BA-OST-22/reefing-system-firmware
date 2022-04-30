/*
 * task_supervision.c
 *
 *  Created on: 5 Apr 2022
 *      Author: Luca
 */

#include "task_supervision.h"
#include "cmsis_os.h"
#include "config/globals.h"
#include "drivers/dcdc.h"
#include "init/init.h"
#include "sensors/battery.h"
#include "usb_device.h"
#include "util/log.h"

BATTERY_SENSOR BATTERY = {
    .source = ADC_SOURCE_BATTERY,
    .multiply = 1.157,
    .add = 0,
};

void task_supervision(void *argument) {

  log_debug("Task Supervisor started");

  bool cli_stared = false;

  osDelay(100);

  float voltage;
  battery_voltage(&BATTERY, &voltage);
  log_info("Battery Voltage: %d mV", (int)voltage);

  uint32_t count = 0;

  uint32_t tick_count = osKernelGetTickCount();
  uint32_t tick_update = osKernelGetTickFreq() / 10;

  while (1) {

    supervisor_mode_e mode =
        osEventFlagsWait(supervisor_mode_id, 0xFF, osFlagsWaitAny, 0);

    if (mode == SUPERVISOR_MODE_SHUTDOWN) {
      log_debug("Task Supervisor stopped");
      if (global_usb_initialized_flag) {
        MX_USB_DEVICE_DeInit();
        global_usb_initialized_flag = false;
        if (cli_stared) {
          deinit_cli();
          cli_stared = false;
        }
      }
      osThreadTerminate(osThreadGetId());
    }

    /* Check the USB detection pin */
    if (HAL_GPIO_ReadPin(USB_DET_GPIO_Port, USB_DET_Pin)) {
      /* Make sure the DCDC converter is never turned on */
      dcdc_disable();

      /* If usb is not initialized yet, do so */
      if (global_usb_initialized_flag == false) {
        global_usb_initialized_flag = true;
        MX_USB_DEVICE_Init();
        log_enable();
      }
    } else {
      if (global_usb_initialized_flag) {
        global_usb_initialized_flag = false;
        MX_USB_DEVICE_DeInit();
        if (cli_stared) {
          deinit_cli();
          cli_stared = false;
        }
      }
    }

    /* Start cli when something was received from the usb interface */
    if (fifo_get_length(&usb_input_fifo) && cli_stared == false) {
      cli_stared = true;
      init_cli();
    }

    battery_voltage(&BATTERY, &voltage);
    if (voltage < 3500.0f) {
      log_error("Battery Low! %dmV", (int)voltage);
    }

    log_info("Battery Voltage:%dmV", (int)voltage);

    if (global_flight_state == RECOVERY) {
      if (count == 20) {
        count = 0;
        osEventFlagsSet(buzzer_event_id, BEEP_RECOVERY);
      } else {
        count++;
      }
    }

    tick_count += tick_update;
    osDelayUntil(tick_count);
  }
}
