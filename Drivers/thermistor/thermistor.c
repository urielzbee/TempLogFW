/*
 * thermistor.c
 *
 *  Created on: Feb 22, 2025
 *      Author: uriel
 */
#include "main.h"
#include "thermistor.h"
#include <math.h>

extern ADC_HandleTypeDef hadc;

static void enableReading(void)
{
	HAL_GPIO_WritePin(TEMP_EN_GPIO_Port, TEMP_EN_Pin, GPIO_PIN_SET);
}

static void disableReading(void)
{
	HAL_GPIO_WritePin(TEMP_EN_GPIO_Port, TEMP_EN_Pin, GPIO_PIN_RESET);
}

static uint16_t thermistor_readThermOut(void)
{
	uint16_t retVal = 0;
	uint16_t buff[3] = {0};

	HAL_ADC_Start(&hadc);  // Start ADC conversion
	HAL_ADC_PollForConversion(&hadc, 1000);

	buff[0] = HAL_ADC_GetValue(&hadc);
	buff[1] = HAL_ADC_GetValue(&hadc);
	//buff[2] = HAL_ADC_GetValue(&hadc);

	HAL_ADC_Stop(&hadc);
	retVal = buff[0];

	return retVal;
}

static uint16_t thermistor_readThermOFST(void)
{
	uint16_t retVal = 0;
	uint16_t buff[3] = {0};

	HAL_ADC_Start(&hadc);  // Start ADC conversion
	HAL_ADC_PollForConversion(&hadc, 1000);

	buff[0] = HAL_ADC_GetValue(&hadc);
	buff[1] = HAL_ADC_GetValue(&hadc);
	//buff[2] = HAL_ADC_GetValue(&hadc);

	HAL_ADC_Stop(&hadc);
	retVal = buff[1];

	return retVal;
}

static uint32_t thermistor_readThermResistance(void)
{
	uint16_t thermOut = 0;
	uint16_t thermOfst = 0;
	uint32_t thermisterOfstVoltage = 0;
	uint32_t thermisterOutVoltage = 0;
	uint32_t thermisterResistance = 0;
	uint32_t R2 = 49900;

	enableReading();
	HAL_Delay(1000);
	thermOut = thermistor_readThermOut();
	HAL_Delay(1000);
	thermOfst = thermistor_readThermOFST();
	disableReading();

	thermisterOutVoltage = (thermOut * 3300) / 4095;
	thermisterOfstVoltage = (thermOfst * 3300) / 4095;

	thermisterResistance = R2 * ((thermisterOutVoltage / thermisterOfstVoltage) - 1);

	return thermisterResistance;
}

uint16_t thermistor_readTemp(void)
{
	double To = 298.15; // Room temperature(Kelvin)
	double Ro = 100000.0; // Resistance at room temperature
	double B = 4261.0; //B = provided by datasheet (Kelvin)
	double Temperature = 0.0;
	uint32_t thermistorResistance = 0;

	thermistorResistance = thermistor_readThermResistance();
	Temperature = (B * To) / (B + (To * log(thermistorResistance / Ro)));
	Temperature = Temperature -273.15;

	return 0;
}

void thermistor_init(void)
{

}
