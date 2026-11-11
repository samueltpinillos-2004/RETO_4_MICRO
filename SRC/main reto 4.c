/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdbool.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

TIM_HandleTypeDef htim4;

/* Definitions for Task_Teclado */
osThreadId_t Task_TecladoHandle;
const osThreadAttr_t Task_Teclado_attributes = {
  .name = "Task_Teclado",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for Task_ADC */
osThreadId_t Task_ADCHandle;
const osThreadAttr_t Task_ADC_attributes = {
  .name = "Task_ADC",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal1,
};
/* Definitions for Task_Logica */
osThreadId_t Task_LogicaHandle;
const osThreadAttr_t Task_Logica_attributes = {
  .name = "Task_Logica",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for Task_Actuadores */
osThreadId_t Task_ActuadoresHandle;
const osThreadAttr_t Task_Actuadores_attributes = {
  .name = "Task_Actuadores",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* USER CODE BEGIN PV */
#define LARGO_CLAVE 4
char claveActual[LARGO_CLAVE + 1] = "1234";   // Contraseña inicial por defecto
char claveIngresada[LARGO_CLAVE + 1] = "";
uint8_t indiceClave = 0;

volatile bool flagNuevaTecla = false;
volatile char teclaPresionada = '\0';
volatile bool modoConfig = false;            // false = Modo Normal, true = Modo Configuración (por el ADC)
volatile uint8_t intentosCierreFallidos = 0; // Conteo para los 3 intentos fallidos de cierre con "0000"

typedef enum {
    ESTADO_ESPERA = 0,       // Azul Puro (Apto para ingresar clave)
    ESTADO_INGRESANDO,       // Morado (Digitando clave en modo normal)
    ESTADO_ABIERTO,          // Verde (Caja abierta, esperando "0000")
    ESTADO_ERROR             // Rojo + Buzzer (Clave incorrecta)
} EstadoCaja_t;

volatile EstadoCaja_t estadoCaja = ESTADO_ESPERA;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM4_Init(void);
void StartTask_Teclado(void *argument);
void StartTask_ADC(void *argument);
void StartTask_Logica(void *argument);
void StartTask_Actuadores(void *argument);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1); // Iniciar PWM para el SG90
  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of Task_Teclado */
  Task_TecladoHandle = osThreadNew(StartTask_Teclado, NULL, &Task_Teclado_attributes);

  /* creation of Task_ADC */
  Task_ADCHandle = osThreadNew(StartTask_ADC, NULL, &Task_ADC_attributes);

  /* creation of Task_Logica */
  Task_LogicaHandle = osThreadNew(StartTask_Logica, NULL, &Task_Logica_attributes);

  /* creation of Task_Actuadores */
  Task_ActuadoresHandle = osThreadNew(StartTask_Actuadores, NULL, &Task_Actuadores_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
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
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 83;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 19999;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */
  HAL_TIM_MspPostInit(&htim4);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(buzzer_pin_GPIO_Port, buzzer_pin_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_0
                          |GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_RESET);

  /*Configure GPIO pin : buzzer_pin_Pin */
  GPIO_InitStruct.Pin = buzzer_pin_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(buzzer_pin_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PD9 PD10 PD11 PD0
                           PD1 PD2 PD3 */
  GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_0
                          |GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : PD4 PD5 PD6 PD7 */
  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void SetColorRGB(bool r, bool g, bool b)
{
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_9,  r ? GPIO_PIN_SET : GPIO_PIN_RESET); // Rojo
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_10, g ? GPIO_PIN_SET : GPIO_PIN_RESET); // Verde
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, b ? GPIO_PIN_SET : GPIO_PIN_RESET); // Azul
}

void MoverServomotor(uint32_t pulso)
{
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, pulso);
}

char EscanearTeclado(void)
{
    char tecla = '\0';
    GPIO_TypeDef* gpio_port = GPIOD;
    uint16_t filas[4] = {GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_2, GPIO_PIN_3};
    uint16_t cols[4]  = {GPIO_PIN_4, GPIO_PIN_5, GPIO_PIN_6, GPIO_PIN_7};

    char teclas_map[4][4] = {
        {'1', '2', '3', 'A'},
        {'4', '5', '6', 'B'},
        {'7', '8', '9', 'C'},
        {'*', '0', '#', 'D'}
    };

    for(int i = 0; i < 4; i++)
    {
        HAL_GPIO_WritePin(gpio_port, filas[0], GPIO_PIN_SET);
        HAL_GPIO_WritePin(gpio_port, filas[1], GPIO_PIN_SET);
        HAL_GPIO_WritePin(gpio_port, filas[2], GPIO_PIN_SET);
        HAL_GPIO_WritePin(gpio_port, filas[3], GPIO_PIN_SET);

        HAL_GPIO_WritePin(gpio_port, filas[i], GPIO_PIN_RESET);

        for(int j = 0; j < 4; j++)
        {
            if(HAL_GPIO_ReadPin(gpio_port, cols[j]) == GPIO_PIN_RESET)
            {
                osDelay(20);
                if(HAL_GPIO_ReadPin(gpio_port, cols[j]) == GPIO_PIN_RESET)
                {
                    tecla = teclas_map[i][j];
                    while(HAL_GPIO_ReadPin(gpio_port, cols[j]) == GPIO_PIN_RESET) {
                        osDelay(10);
                    }
                    return tecla;
                }
            }
        }
    }
    return '\0';
}
/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartTask_Teclado */
/**
  * @brief  Function implementing the Task_Teclado thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartTask_Teclado */
void StartTask_Teclado(void *argument)
{
  /* USER CODE BEGIN 5 */
  for(;;)
  {
    char tecla = EscanearTeclado();
    if (tecla != '\0')
    {
      teclaPresionada = tecla;
      flagNuevaTecla = true;
    }
    osDelay(30);
  }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_StartTask_ADC */
/**
  * @brief Function implementing the Task_ADC thread.
  * @param argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartTask_ADC */
void StartTask_ADC(void *argument)
{
  /* USER CODE BEGIN StartTask_ADC */
  for(;;)
  {
    HAL_ADC_Start(&hadc1);
    if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK)
    {
      uint32_t valADC = HAL_ADC_GetValue(&hadc1);
      if (valADC > 2048) {
        modoConfig = true;
      } else {
        modoConfig = false;
      }
    }
    HAL_ADC_Stop(&hadc1);
    osDelay(200);
  }
  /* USER CODE END StartTask_ADC */
}

/* USER CODE BEGIN Header_StartTask_Logica */
/**
  * @brief Function implementing the Task_Logica thread.
  * @param argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartTask_Logica */
void StartTask_Logica(void *argument)
{
  /* USER CODE BEGIN StartTask_Logica */
  for(;;)
  {
    if (modoConfig)
    {
      if (estadoCaja == ESTADO_ESPERA)
      {
        indiceClave = 0;
        memset(claveIngresada, 0, sizeof(claveIngresada));

        while (modoConfig)
        {
          if (flagNuevaTecla)
          {
            flagNuevaTecla = false;
            char t = teclaPresionada;
            if (t >= '0' && t <= '9')
            {
              claveIngresada[indiceClave++] = t;
              if (indiceClave == 4)
              {
                claveIngresada[4] = '\0';
                strcpy(claveActual, claveIngresada);

                osDelay(1500); // Pausa para mostrar Cian (confirmación)
                indiceClave = 0;
                break;
              }
            }
          }
          osDelay(50);
        }
      }
    }
    else
    {
      switch(estadoCaja)
      {
        case ESTADO_ESPERA:
          if (flagNuevaTecla)
          {
            flagNuevaTecla = false;
            char t = teclaPresionada;
            if (t >= '0' && t <= '9')
            {
              claveIngresada[indiceClave++] = t;
              estadoCaja = ESTADO_INGRESANDO;
              if (indiceClave == 4)
              {
                claveIngresada[4] = '\0';
                indiceClave = 0;

                if (strcmp(claveIngresada, claveActual) == 0)
                {
                  estadoCaja = ESTADO_ABIERTO;
                  intentosCierreFallidos = 0;
                }
                else
                {
                  estadoCaja = ESTADO_ERROR;
                }
              }
            }
          }
          break;

        case ESTADO_INGRESANDO:
          if (flagNuevaTecla)
          {
            flagNuevaTecla = false;
            char t = teclaPresionada;
            if (t >= '0' && t <= '9')
            {
              claveIngresada[indiceClave++] = t;
              if (indiceClave == 4)
              {
                claveIngresada[4] = '\0';
                indiceClave = 0;

                if (strcmp(claveIngresada, claveActual) == 0)
                {
                  estadoCaja = ESTADO_ABIERTO;
                  intentosCierreFallidos = 0;
                }
                else
                {
                  estadoCaja = ESTADO_ERROR;
                }
              }
            }
          }
          break;

        case ESTADO_ABIERTO:
          if (flagNuevaTecla)
          {
            flagNuevaTecla = false;
            char t = teclaPresionada;
            if (t >= '0' && t <= '9')
            {
              claveIngresada[indiceClave++] = t;
              if (indiceClave == 4)
              {
                claveIngresada[4] = '\0';
                indiceClave = 0;

                if (strcmp(claveIngresada, "0000") == 0)
                {
                  estadoCaja = ESTADO_ESPERA;
                  intentosCierreFallidos = 0;
                }
                else
                {
                  intentosCierreFallidos++;
                  if (intentosCierreFallidos >= 3)
                  {
                    intentosCierreFallidos = 0;
                    estadoCaja = ESTADO_ESPERA;
                  }
                }
              }
            }
          }
          break;

        case ESTADO_ERROR:
          osDelay(2000);
          estadoCaja = ESTADO_ESPERA;
          indiceClave = 0;
          break;
      }
    }

    osDelay(50);
  }
  /* USER CODE END StartTask_Logica */
}

/* USER CODE BEGIN Header_StartTask_Actuadores */
/**
  * @brief Function implementing the Task_Actuadores thread.
  * @param argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartTask_Actuadores */
void StartTask_Actuadores(void *argument)
{
  /* USER CODE BEGIN StartTask_Actuadores */
  for(;;)
  {
    if (modoConfig)
    {
      SetColorRGB(true, true, false); // Amarillo
      MoverServomotor(1250);
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);
    }
    else
    {
      switch(estadoCaja)
      {
        case ESTADO_ESPERA:
          SetColorRGB(false, false, true); // Azul Puro
          MoverServomotor(1250);
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
          break;

        case ESTADO_INGRESANDO:
          SetColorRGB(true, false, true); // Morado
          MoverServomotor(1250);
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);
          break;

        case ESTADO_ABIERTO:
          SetColorRGB(false, true, false); // Verde
          MoverServomotor(2500);
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);
          break;

        case ESTADO_ERROR:
          SetColorRGB(true, false, false); // Rojo
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET);
          osDelay(2000);
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);
          break;
      }
    }

    osDelay(100);
  }
  /* USER CODE END StartTask_Actuadores */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6)
  {
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
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
