/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED0_Pin GPIO_PIN_1
#define LED0_GPIO_Port GPIOA
#define LED1_Pin GPIO_PIN_2
#define LED1_GPIO_Port GPIOA
#define FLASH_nCS_Pin GPIO_PIN_4
#define FLASH_nCS_GPIO_Port GPIOA
#define FLASH_SCK_Pin GPIO_PIN_5
#define FLASH_SCK_GPIO_Port GPIOA
#define FLASH_MISO_Pin GPIO_PIN_6
#define FLASH_MISO_GPIO_Port GPIOA
#define FLASH_MOSI_Pin GPIO_PIN_7
#define FLASH_MOSI_GPIO_Port GPIOA
#define FLASH_nWP_Pin GPIO_PIN_0
#define FLASH_nWP_GPIO_Port GPIOB
#define TEMP_EN_Pin GPIO_PIN_1
#define TEMP_EN_GPIO_Port GPIOB
#define THERM_OUT_Pin GPIO_PIN_2
#define THERM_OUT_GPIO_Port GPIOB
#define THERM_OFST_Pin GPIO_PIN_10
#define THERM_OFST_GPIO_Port GPIOB
#define MCO_Pin GPIO_PIN_8
#define MCO_GPIO_Port GPIOA
#define BTN_1_Pin GPIO_PIN_9
#define BTN_1_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
