/*
 * adc.c
 *
 *  Created on: Mar 31, 2022
 *      Author: luca.jost
 */

#include "adc.h"

extern ADC_HandleTypeDef hadc1;

void ADC_Select(adc_source_t channel)
{
	ADC_ChannelConfTypeDef sConfig = {0};
	  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
	  */
	  sConfig.Channel = (uint32_t)channel;
	  sConfig.Rank = 1;
	  sConfig.SamplingTime = ADC_SAMPLETIME_56CYCLES;
	  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK){
	    return;
	  }
}

uint32_t ADC_Get(adc_source_t channel){
	uint32_t value;
	ADC_Select(channel);
	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, 1000);
	value = HAL_ADC_GetValue(&hadc1);
	HAL_ADC_Stop(&hadc1);
	return value;
}
