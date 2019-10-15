/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2018 STMicroelectronics International N.V.
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice,
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other
  *    contributors to this software may be used to endorse or promote products
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under
  *    this license is void and will automatically terminate your rights under
  *    this license.
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

  /* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

/* USER CODE BEGIN Includes */
#include "stm32f1xx_hal.h"
#include "eeprom.h"
#include "lcd5110.h"
#include "ds18b20.h"

/* USER CODE END Includes */

/* Variables -----------------------------------------------------------------*/
osThreadId Task01MainHandle;
osThreadId Task02LCDHandle;
osThreadId Task03LedDiodHandle;
osThreadId Task04ButtonHandle;
osThreadId Task05OptionHandle;
osThreadId Task06AmperHandle;
osThreadId Task07AutoModeHandle;

/* USER CODE BEGIN Variables */
LCD5110_display lcd1;

bool relaystatus = false;
bool autoMode = false;
bool handMode = false;
bool on = false, off = false;
uint32_t param[] = { 3000, 5000 };                            //на начало создания прибора
uint32_t *workTime = &param[0], *sleepTime = &param[1];
uint16_t delayLedGreed;
uint32_t secTick = 0, lastTimeOn = 0, lastTimeOff = 0, timeTemp = 0, temper;
uint32_t lastTouch, starts = 0;
uint32_t hours = 0, min = 0, sec = 0, timeToOn, uptime;

/* USER CODE END Variables */
/* Function prototypes -------------------------------------------------------*/
void MainTask(void const * argument);
void LcdOutTask(void const * argument);
void LedDiodTask(void const * argument);
void ButtonTask(void const * argument);
void OptionTask(void const * argument);
void AmperTask(void const * argument);
void AutoModeTask(void const * argument);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* USER CODE BEGIN FunctionPrototypes */
void options();
void suspendTasks();
void resumeTasks();
void lcdTime(uint32_t);
/* USER CODE END FunctionPrototypes */

/* Hook prototypes */

/* Init FreeRTOS */

void MX_FREERTOS_Init(void) {
	/* USER CODE BEGIN Init */
	lcd1.hw_conf.spi_handle = &hspi1;
	lcd1.hw_conf.spi_cs_pin = LCD1_CS_Pin;
	lcd1.hw_conf.spi_cs_port = LCD1_CS_GPIO_Port;
	lcd1.hw_conf.rst_pin = LCD1_RST_Pin;
	lcd1.hw_conf.rst_port = LCD1_RST_GPIO_Port;
	lcd1.hw_conf.dc_pin = LCD1_DC_Pin;
	lcd1.hw_conf.dc_port = LCD1_DC_GPIO_Port;
	lcd1.def_scr = lcd5110_def_scr;
	LCD5110_init(&lcd1.hw_conf, LCD5110_NORMAL_MODE, 0x40, 2, 3);

	//загрузка параметров
	EE_Reads(0, 2, param);

	/* USER CODE END Init */

	/* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
	/* USER CODE END RTOS_MUTEX */

	/* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
	/* USER CODE END RTOS_SEMAPHORES */

	/* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
	/* USER CODE END RTOS_TIMERS */

	/* Create the thread(s) */
	/* definition and creation of Task01Main */
	osThreadDef(Task01Main, MainTask, osPriorityIdle, 0, 128);
	Task01MainHandle = osThreadCreate(osThread(Task01Main), NULL);

	/* definition and creation of Task02LCD */
	osThreadDef(Task02LCD, LcdOutTask, osPriorityIdle, 0, 256);
	Task02LCDHandle = osThreadCreate(osThread(Task02LCD), NULL);

	/* definition and creation of Task03LedDiod */
	osThreadDef(Task03LedDiod, LedDiodTask, osPriorityIdle, 0, 48);
	Task03LedDiodHandle = osThreadCreate(osThread(Task03LedDiod), NULL);

	/* definition and creation of Task04Button */
	osThreadDef(Task04Button, ButtonTask, osPriorityIdle, 0, 128);
	Task04ButtonHandle = osThreadCreate(osThread(Task04Button), NULL);

	/* definition and creation of Task05Option */
	osThreadDef(Task05Option, OptionTask, osPriorityIdle, 0, 256);
	Task05OptionHandle = osThreadCreate(osThread(Task05Option), NULL);

	/* definition and creation of Task06Amper */
	osThreadDef(Task06Amper, AmperTask, osPriorityIdle, 0, 8);
	Task06AmperHandle = osThreadCreate(osThread(Task06Amper), NULL);

	/* definition and creation of Task07AutoMode */
	osThreadDef(Task07AutoMode, AutoModeTask, osPriorityIdle, 0, 128);
	Task07AutoModeHandle = osThreadCreate(osThread(Task07AutoMode), NULL);

	/* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
	Ds18b20_Init(osPriorityNormal);

	/* USER CODE END RTOS_THREADS */

	/* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
	/* USER CODE END RTOS_QUEUES */
}

/* MainTask function */
void MainTask(void const * argument)
{

	/* USER CODE BEGIN MainTask */
	/* Infinite loop */
	for (;;)
	{
		if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_10))  	//если прижата кнопка auto
		{
			delayLedGreed = 480;              //изменил время мерцания зеленого светодиода
			autoMode = true;
		}
		else
		{
			delayLedGreed = 1480;
			autoMode = false;
		}

		if (!autoMode && !handMode && relaystatus && off) 	// авторежим и ручной режим выключены, а реле осталось включенным
		{
			relaystatus = false;    				//оффни его!
			off = false;
			lastTimeOff = HAL_GetTick();
		}

		if (relaystatus && on)
		{

			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET);		 //включи реле, если флаг задан
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_SET);		 //и светодиод
			lastTimeOn = HAL_GetTick();
			on = false;
			 starts++;
		}

		else if (off)
		{
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);		  		//иначе, выключи реле
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_RESET);			//и светодиод
			lastTimeOff = HAL_GetTick();
			off = false;
		}


		//  LCD5110_refresh(&lcd1);
		osDelay(100);
	}
	/* USER CODE END MainTask */
}

/* LcdOutTask function */
void LcdOutTask(void const * argument)
{
	/* USER CODE BEGIN LcdOutTask */
	/* Infinite loop */
	for (;;)
	{
		LCD5110_clear_scr(&lcd1);
		
		LCD5110_set_cursor(0, 0, &lcd1);
		LCD5110_printf(&lcd1, BLACK, "%s", "Вкл ");
		
		lcdTime(HAL_GetTick());         //uptime

		LCD5110_set_cursor(0, 10, &lcd1);
		LCD5110_printf(&lcd1, BLACK, "%s", "Пусков ");
		LCD5110_printf(&lcd1, BLACK, "%i", starts);
		
		LCD5110_set_cursor(0, 20, &lcd1);
		LCD5110_printf(&lcd1, BLACK, "%s", "Спать ");
		lcdTime(*sleepTime + lastTimeOff - HAL_GetTick());

		LCD5110_set_cursor(0, 30, &lcd1);

		LCD5110_set_cursor(0, 40, &lcd1);
		LCD5110_printf(&lcd1, BLACK, "%s", "Temper~");
		LCD5110_set_cursor(60, 40, &lcd1);
		LCD5110_printf(&lcd1, BLACK, "%2.1f", ds18b20[0].Temperature); 
		
		LCD5110_refresh(&lcd1);
		osDelay(333);
	}
	/* USER CODE END LcdOutTask */
}

/* LedDiodTask function */
void LedDiodTask(void const * argument)
{
	/* USER CODE BEGIN LedDiodTask */
	/* Infinite loop */
	for (;;)
	{
		//HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
		osDelay(20);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
		osDelay(delayLedGreed);

		osDelay(20);
	}
	/* USER CODE END LedDiodTask */
}

/* ButtonTask function */
void ButtonTask(void const * argument)
{
	/* USER CODE BEGIN ButtonTask */
	/* Infinite loop */
	for (;;)
	{
		if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_11))     //вручную вкл реле
		{
			relaystatus = true;
			on = true;
			handMode = true;
			vTaskSuspend(Task07AutoModeHandle);

			do
			{
				osDelay(50);								// а пока клавиша прижата, сделай что нить другое, полезное

			} while (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_11));
			relaystatus = false;
			// HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);
			handMode = false;
			off = true;
			vTaskResume(Task07AutoModeHandle);
		}

		osDelay(30);
	}
	/* USER CODE END ButtonTask */
}

/* OptionTask function */
void OptionTask(void const * argument)
{
	/* USER CODE BEGIN OptionTask */
	/* Infinite loop */
	for (;;)
	{
		if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_7)) 
		{
			suspendTasks();
			lastTouch = HAL_GetTick();
			options();

			//туточки запись во флеш
			EE_Writes(0, 2, param);

			HAL_Delay(200);
			resumeTasks();
		}
		osDelay(30);
	}
	/* USER CODE END OptionTask */
}

/* AmperTask function */
void AmperTask(void const * argument)
{
	/* USER CODE BEGIN AmperTask */
	/* Infinite loop */
	for (;;)
	{

		osDelay(1000);
		vTaskDelete(Task06AmperHandle);
		osDelay(1);
	}
	/* USER CODE END AmperTask */
}

/* AutoModeTask function */
void AutoModeTask(void const * argument)
{
	/* USER CODE BEGIN AutoModeTask */
	/* Infinite loop */
	for (;;)
	{
		if (autoMode && !handMode)
		{
			timeTemp = HAL_GetTick() - lastTimeOff;

			if ((timeTemp > *sleepTime) /*&& (ds18b20[0].Temperature > (-2))*/)
			{
				relaystatus = true;
				on = true;
				osDelay(*workTime);

				relaystatus = false;
				off = true;
				osDelay(*sleepTime);
			}
		}

		osDelay(50);
	}
	/* USER CODE END AutoModeTask */
}

/* USER CODE BEGIN Application */
void suspendTasks()
{
	vTaskSuspend(Task01MainHandle);
	vTaskSuspend(Task02LCDHandle);
	vTaskSuspend(Task03LedDiodHandle);
	vTaskSuspend(Task04ButtonHandle);
	//vTaskSuspend(Task06AmperHandle);
	vTaskSuspend(Task07AutoModeHandle);
	vTaskSuspend(Ds18b20Handle);
}

void resumeTasks()
{
	vTaskResume(Task01MainHandle);
	vTaskResume(Task02LCDHandle);
	vTaskResume(Task03LedDiodHandle);
	vTaskResume(Task04ButtonHandle);
	//vTaskResume(Task06AmperHandle);
	vTaskResume(Task07AutoModeHandle);
	vTaskResume(Ds18b20Handle);
}
void options()
{
	do
	{
		while (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_6))               //кнопка добавления
		{
			LCD5110_clear_scr(&lcd1);
			LCD5110_set_cursor(0, 0, &lcd1);
			LCD5110_printf(&lcd1, BLACK, "%s", "Работа: ");

			*workTime += 1000;

			//LCD5110_set_cursor(25, 27, &lcd1);
			LCD5110_printf(&lcd1, BLACK, "%i", *workTime / 1000);
			LCD5110_printf(&lcd1, BLACK, "%s", "сек");

			LCD5110_refresh(&lcd1);
			HAL_Delay(100);
			lastTouch = HAL_GetTick();
		}

		while (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_5))			////кнопка убавления
		{
			LCD5110_clear_scr(&lcd1);
			LCD5110_set_cursor(0, 0, &lcd1);
			LCD5110_printf(&lcd1, BLACK, "%s", "Работа: ");

			*workTime -= 1000;
			if (*workTime < 2000)									//включение короче 2 секунд не нужно
				*workTime = 2000;
			//LCD5110_set_cursor(25, 27, &lcd1);
			LCD5110_printf(&lcd1, BLACK, "%i", *workTime / 1000);
			LCD5110_printf(&lcd1, BLACK, "%s", "сек");
			LCD5110_refresh(&lcd1);
			HAL_Delay(100);
			lastTouch = HAL_GetTick();
		}


		LCD5110_clear_scr(&lcd1);
		LCD5110_set_cursor(0, 0, &lcd1);
		LCD5110_printf(&lcd1, BLACK, "%s", "Работа: ");
		LCD5110_printf(&lcd1, BLACK, "%i", (*workTime / 1000));
		LCD5110_printf(&lcd1, BLACK, "%s", "сек");
		LCD5110_refresh(&lcd1);

		HAL_Delay(150);

		//---------------------------сон--------------------------------------------------
		while (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_7))
		{
			lastTouch = HAL_GetTick();
			do
			{
				LCD5110_clear_scr(&lcd1);
				LCD5110_set_cursor(0, 0, &lcd1);
				LCD5110_printf(&lcd1, BLACK, "%s", "Спать: ");
				LCD5110_printf(&lcd1, BLACK, "%i", (*sleepTime / 60000));
				LCD5110_printf(&lcd1, BLACK, "%s", "мин");
				LCD5110_refresh(&lcd1);
				HAL_Delay(100);

				while (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_6))               //кнопка добавления
				{
					LCD5110_clear_scr(&lcd1);
					LCD5110_set_cursor(0, 0, &lcd1);
					LCD5110_printf(&lcd1, BLACK, "%s", "Спать: ");

					*sleepTime += 60000;
					LCD5110_printf(&lcd1, BLACK, "%i", *sleepTime / 60000);
					LCD5110_printf(&lcd1, BLACK, "%s", "мин");

					LCD5110_refresh(&lcd1);
					HAL_Delay(100);
					lastTouch = HAL_GetTick();
				}

				while (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_5))			////кнопка убавления
				{
					LCD5110_clear_scr(&lcd1);
					LCD5110_set_cursor(0, 0, &lcd1);
					LCD5110_printf(&lcd1, BLACK, "%s", "Спать: ");

					*sleepTime -= 60000;
					if (*sleepTime < 60000)									//выключение короче 3 секунд не нужно
						*sleepTime = 60000;
					LCD5110_printf(&lcd1, BLACK, "%i", *sleepTime / 60000);
					LCD5110_printf(&lcd1, BLACK, "%s", "мин");

					LCD5110_refresh(&lcd1);
					HAL_Delay(100);
					lastTouch = HAL_GetTick();
				}

				if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_7))
				{
					break;
				}

			} while ((lastTouch + 5000) > HAL_GetTick());

			break;
		}
		//---------------------------------------------------------------	   		 


	} while ((lastTouch + 5000) > HAL_GetTick());
}

void lcdTime(uint32_t a)
{
	uint32_t _hours = 0, _min = 0, _sec = 0;
	do
	{
		if (a >= 3600000)
		{
			_hours++;
			a -= 3600000;
			continue;
		}
		else if (a >= 60000)
		{
			_min++;
			a -= 60000;
			continue;
		}
		else if (a >= 1000)
		{
			_sec++;
			a -= 1000;
			continue;
		}

	} while (a >= 1000);

	LCD5110_printf(&lcd1, BLACK, "%i", _hours);
	LCD5110_printf(&lcd1, BLACK, "%c", 149);     // точка посередине в ASCII
	LCD5110_printf(&lcd1, BLACK, "%i", _min);
	LCD5110_printf(&lcd1, BLACK, "%c", 149);
	LCD5110_printf(&lcd1, BLACK, "%i", _sec);
	_hours = _min = _sec = 0;

	
}




/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
