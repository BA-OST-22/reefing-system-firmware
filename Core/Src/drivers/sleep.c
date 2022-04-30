#include "sleep.h"
#include "config/globals.h"
#include "drivers/led.h"
#include "init/init.h"
#include "target/target.h"

void go_to_sleep(wakeup_e source) {

  led_off();
  while ((HAL_GPIO_ReadPin(BUTTON_GPIO_Port, BUTTON_Pin) == 0))
    ;

  HAL_Delay(10);

  __HAL_GPIO_EXTI_CLEAR_IT(BUTTON_Pin);
  __HAL_GPIO_EXTI_CLEAR_IT(IMU_INT1_Pin);

  if (source == WAKEUP_BUTTON) {
    HAL_NVIC_EnableIRQ(EXTI2_IRQn);
  } else if (source == WAKEUP_ACCEL) {
    osEventFlagsSet(sensor_mode_id, SENSOR_MODE_WAKEUP);
    HAL_Delay(10);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
  } else {
    osEventFlagsSet(sensor_mode_id, SENSOR_MODE_WAKEUP);
    HAL_Delay(10);
    HAL_NVIC_EnableIRQ(EXTI2_IRQn);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
  }

  /* Disable Systick interrupt */
  HAL_SuspendTick();

  /* Go to sleep */
  HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);

  /* Return from sleep */
  HAL_NVIC_DisableIRQ(EXTI2_IRQn);
  HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);

  SystemClock_Config();
  HAL_ResumeTick();

  if (source == WAKEUP_ACCEL || source == WAKEUP_BOTH) {
    /* Configure IMU to go back to continous mode */
    osEventFlagsSet(sensor_mode_id, SENSOR_MODE_CONTINUOUS);
  }
}

void wake_up() {
  /* Clear Wake Up Flag */
  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  if ((GPIO_Pin == BUTTON_Pin) || (GPIO_Pin == IMU_INT1_Pin)) {
    wake_up();
  }
}
