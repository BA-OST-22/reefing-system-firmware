#include "main.h"
#include "sleep.h"

void go_to_sleep(){

	HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);

	while((HAL_GPIO_ReadPin(BUTTON_GPIO_Port, BUTTON_Pin) == 0));
	HAL_Delay(10);
	HAL_NVIC_EnableIRQ(EXTI2_IRQn);
	/* Disable Systick interrupt */
	HAL_SuspendTick();

    /* Go to sleep */
    HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);

    /* Return from sleep */
    SystemClock_Config();
    HAL_ResumeTick();

}

void wake_up(){
	/* Clear Wake Up Flag */
	__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
}



