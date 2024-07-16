/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  * @author         : Achraf Hentati
  * @date           : 16-07-2024
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "command_util.h"
#include "queue.h"
#include <string.h>
#include <stdio.h>

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart2;

QueueHandle_t uart2_BytesReceived = NULL;
QueueHandle_t print_queue = NULL;

TaskHandle_t blink_red_handle = NULL;
TaskHandle_t blink_green_handle = NULL;
TaskHandle_t blink_orange_handle = NULL;
TaskHandle_t blink_blue_handle = NULL;

int g_red_delay = 0;
int g_blue_delay = 0;
int g_orange_delay = 0;
int g_green_delay = 0;

uint8_t rcvByte;
int test = tskIDLE_PRIORITY + 3;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);

void vCommandHandlerTask(void *pvParameters);
void vPrintTask(void *pvParameters);
void vBlinkGreenTask(void *pvParameters);
void vBlinkBlueTask(void *pvParameters);
void vBlinkRedTask(void *pvParameters);
void vBlinkOrangeTask(void *pvParameters);

void ledOnHandler(Command cmd);
void ledOffHandler(Command cmd);
void ledToggleHandler(Command cmd);
void ledBlinkHandler(Command cmd);

void BlinkLed(int pin, int delay);
void deleteBlinkTask(LEDColor color);

/* Main function -------------------------------------------------------------*/
int main(void)
{
  /* Initialize the HAL Library */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();

  /* Start UART reception in interrupt mode */
  HAL_UART_Receive_IT(&huart2, &rcvByte, 1);

  /* Create queues */
  uart2_BytesReceived = xQueueCreate(20, sizeof(char));
  print_queue = xQueueCreate(10, sizeof(char[40]));

  /* Send welcome message */
  xQueueSend(print_queue, "Welcome to UART CLI demo application\n", portMAX_DELAY);

  /* Create tasks */
  xTaskCreate(vCommandHandlerTask, "commandHandler", 200, NULL, tskIDLE_PRIORITY + 3, NULL);
  xTaskCreate(vPrintTask, "printHandler", 1000, NULL, tskIDLE_PRIORITY + 3, NULL);

  /* Start the scheduler */
  vTaskStartScheduler();

  /* Infinite loop */
  while (1)
  {
  }
}

/* Task to handle incoming UART commands -------------------------------------*/
void vCommandHandlerTask(void *pvParameters) {
    int ind = 0;
    char rcvStr[20];
    Command cmd = initCommand();

    while (1) {
        /* Receive the next byte and store it in rcvStr */
        xQueueReceive(uart2_BytesReceived, &rcvStr[ind++], portMAX_DELAY);

        /* Check if the received byte is a newline character */
        if (rcvStr[ind-1] == '\n') {
            /* Terminate the string properly */
            rcvStr[ind - 1] = '\0';

            /* Reset the index for the next command */
            ind = 0;

            /* Parse the received command */
            cmd = parseReceivedCommand(rcvStr);

            /* Clear the receive buffer */
            memset(rcvStr, 0, sizeof(rcvStr));

            /* Check if the command is valid */
            if (cmd.color != 9 && cmd.func != 9) {
                switch (cmd.func) {
                    case ON:
                        ledOnHandler(cmd);
                        break;
                    case OFF:
                        ledOffHandler(cmd);
                        break;
                    case TOGGLE:
                        ledToggleHandler(cmd);
                        break;
                    case BLINK:
                        ledBlinkHandler(cmd);
                        break;
                }
            } else {
                xQueueSend(print_queue, "Invalid command\n", portMAX_DELAY);
            }

            /* Reset command information */
            resetCommand(&cmd);
        }
    }
}

/* Task to print messages via UART -------------------------------------------*/
void vPrintTask(void *pvParameters)
{
    char strToPrint[50];

    while (1) {
        if (xQueueReceive(print_queue, &strToPrint, portMAX_DELAY) == pdPASS) {
            HAL_UART_Transmit(&huart2, (uint8_t *)strToPrint, strlen(strToPrint), HAL_MAX_DELAY);
        }
    }
}

/* Task to blink green LED ---------------------------------------------------*/
void vBlinkGreenTask(void *pvParameters)
{
    int delay = (int)pvParameters;
    g_green_delay = delay;

    while (1) {
        BlinkLed(GPIO_PIN_12, g_green_delay);
    }
}

/* Task to blink orange LED --------------------------------------------------*/
void vBlinkOrangeTask(void *pvParameters)
{
    int delay = (int)pvParameters;
    g_orange_delay = delay;

    while (1) {
        BlinkLed(GPIO_PIN_13, g_orange_delay);
    }
}

/* Task to blink red LED -----------------------------------------------------*/
void vBlinkRedTask(void *pvParameters)
{
    int delay = (int)pvParameters;
    g_red_delay = delay;

    while (1) {
        BlinkLed(GPIO_PIN_14, g_red_delay);
    }
}

/* Task to blink blue LED ----------------------------------------------------*/
void vBlinkBlueTask(void *pvParameters)
{
    int delay = (int)pvParameters;
    g_blue_delay = delay;

    while (1) {
        BlinkLed(GPIO_PIN_15, g_blue_delay);
    }
}

/* Handler for LED ON command ------------------------------------------------*/
void ledOnHandler(Command cmd)
{
    char strToPrint[50];
    deleteBlinkTask(cmd.color);

    switch (cmd.color) {
        case RED:
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, SET);
            sprintf(strToPrint, "Red LED ON\n");
            break;
        case GREEN:
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, SET);
            sprintf(strToPrint, "Green LED ON\n");
            break;
        case BLUE:
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, SET);
            sprintf(strToPrint, "Blue LED ON\n");
            break;
        case ORANGE:
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, SET);
            sprintf(strToPrint, "Orange LED ON\n");
            break;
    }
    xQueueSend(print_queue, strToPrint, portMAX_DELAY);
}

/* Handler for LED OFF command -----------------------------------------------*/
void ledOffHandler(Command cmd)
{
    char strToPrint[50];
    deleteBlinkTask(cmd.color);

    switch (cmd.color) {
        case RED:
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, RESET);
            sprintf(strToPrint, "Red LED OFF\n");
            break;
        case GREEN:
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, RESET);
            sprintf(strToPrint, "Green LED OFF\n");
            break;
        case BLUE:
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, RESET);
            sprintf(strToPrint, "Blue LED OFF\n");
            break;
        case ORANGE:
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, RESET);
            sprintf(strToPrint, "Orange LED OFF\n");
            break;
    }
    xQueueSend(print_queue, strToPrint, portMAX_DELAY);
}

/* Handler for LED TOGGLE command --------------------------------------------*/
void ledToggleHandler(Command cmd)
{
    char strToPrint[50];
    deleteBlinkTask(cmd.color);

    switch (cmd.color) {
        case RED:
            HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_14);
            sprintf(strToPrint, "Red LED TOGGLED\n");
            break;
        case GREEN:
            HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_12);
            sprintf(strToPrint, "Green LED TOGGLED\n");
            break;
        case BLUE:
            HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_15);
            sprintf(strToPrint, "Blue LED TOGGLED\n");
            break;
        case ORANGE:
            HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_13);
            sprintf(strToPrint, "Orange LED TOGGLED\n");
            break;
    }
    xQueueSend(print_queue, strToPrint, portMAX_DELAY);
}

/* Handler for LED BLINK command ---------------------------------------------*/
void ledBlinkHandler(Command cmd)
{
    char strToPrint[50];
    char delayStr[10];

    sprintf(delayStr, "%d", cmd.delay);

    switch (cmd.color) {
        case GREEN:
            if (!blink_green_handle) {
                xTaskCreate(vBlinkGreenTask, "BlinkGreen", 100, (void *)(intptr_t)cmd.delay, tskIDLE_PRIORITY + 3, &blink_green_handle);
            } else {
                g_green_delay = cmd.delay;
            }
            snprintf(strToPrint, sizeof(strToPrint), "Green LED BLINK with %s ms delay\n", delayStr);
            break;
        case ORANGE:
            if (!blink_orange_handle) {
                xTaskCreate(vBlinkOrangeTask, "BlinkOrange", 100, (void *)(intptr_t)cmd.delay, tskIDLE_PRIORITY + 3, &blink_orange_handle);
            } else {
                g_orange_delay = cmd.delay;
            }
            snprintf(strToPrint, sizeof(strToPrint), "Orange LED BLINK with %s ms delay\n", delayStr);
            break;
        case RED:
            if (!blink_red_handle) {
                xTaskCreate(vBlinkRedTask, "BlinkRed", 100, (void *)(intptr_t)cmd.delay, tskIDLE_PRIORITY + 3, &blink_red_handle);
            } else {
                g_red_delay = cmd.delay;
            }
            snprintf(strToPrint, sizeof(strToPrint), "Red LED BLINK with %s ms delay\n", delayStr);
            break;
        case BLUE:
            if (!blink_blue_handle) {
                xTaskCreate(vBlinkBlueTask, "BlinkBlue", 100, (void *)(intptr_t)cmd.delay, tskIDLE_PRIORITY + 3, &blink_blue_handle);
            } else {
                g_blue_delay = cmd.delay;
            }
            snprintf(strToPrint, sizeof(strToPrint), "Blue LED BLINK with %s ms delay\n", delayStr);
            break;
    }
    xQueueSend(print_queue, strToPrint, portMAX_DELAY);
}

/* Function to handle LED blinking -------------------------------------------*/
void BlinkLed(int pin, int delay)
{
    HAL_GPIO_TogglePin(GPIOD, pin);
    vTaskDelay(delay);
}

/* Function to delete a specific blink task ----------------------------------*/
void deleteBlinkTask(LEDColor color)
{
    switch (color) {
        case RED:
            if (blink_red_handle) {
                vTaskDelete(blink_red_handle);
                blink_red_handle = NULL;
            }
            break;
        case GREEN:
            if (blink_green_handle) {
                vTaskDelete(blink_green_handle);
                blink_green_handle = NULL;
            }
            break;
        case BLUE:
            if (blink_blue_handle) {
                vTaskDelete(blink_blue_handle);
                blink_blue_handle = NULL;
            }
            break;
        case ORANGE:
            if (blink_orange_handle) {
                vTaskDelete(blink_orange_handle);
                blink_orange_handle = NULL;
            }
            break;
    }
}

/* Callback for UART reception complete --------------------------------------*/

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2) {

	  portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	  xQueueSendFromISR(uart2_BytesReceived, &rcvByte, &xHigherPriorityTaskWoken);
	  HAL_UART_Receive_IT(&huart2, &rcvByte, 1);
	  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

    }
}


/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_RESET);

  /*Configure GPIO pins : PD12 PD13 PD14 PD15 */
  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */


  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
