/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Pan/Tilt with CMSIS-RTOS v2, UART2 DMA IDLE RX
 ******************************************************************************
 * @attention
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>

#ifndef CMD_BUF_LEN
#define CMD_BUF_LEN 16 // typedef에서 필요하므로 선 정의
#endif
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct
{
  char line[CMD_BUF_LEN];
} cmd_msg_t;
typedef struct
{
  int pan;
  int tilt;
} servo_cmd_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#ifndef RX_BUF_LEN
#define RX_BUF_LEN 128
#endif

#ifndef CMD_BUF_LEN
#define CMD_BUF_LEN 16
#endif

#define SERVO_MIN_US 500
#define SERVO_MAX_US 2500
#define SERVO_FULL_DEG 180

// 각도 범위(요청 사양에 맞춤)
#define PAN_MIN_DEG 0
#define PAN_MAX_DEG 90 // 팬은 0..90°
#define TILT_MIN_DEG 0
#define TILT_MAX_DEG 180 // 틸트는 0..180°

#define L1_STEP_DEG 1
#define L2_STEP_DEG 1
#define L3_STEP_DEG 1

#define EV_TICK (1u << 0) // 10ms 틱
#define EV_RX (1u << 1)   // 수신 발생
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart2_rx;

/* Definitions for servo_task */
osThreadId_t servo_taskHandle;
const osThreadAttr_t servo_task_attributes = {
    .name = "servo_task",
    .stack_size = 256 * 4,
    .priority = (osPriority_t)osPriorityNormal,
};
/* Definitions for rx_task */
osThreadId_t rx_taskHandle;
const osThreadAttr_t rx_task_attributes = {
    .name = "rx_task",
    .stack_size = 256 * 4,
    .priority = (osPriority_t)osPriorityLow,
};
/* Definitions for cmd_task */
osThreadId_t cmd_taskHandle;
const osThreadAttr_t cmd_task_attributes = {
    .name = "cmd_task",
    .stack_size = 256 * 4,
    .priority = (osPriority_t)osPriorityLow,
};
/* Definitions for idle_task */
osThreadId_t idle_taskHandle;
const osThreadAttr_t idle_task_attributes = {
    .name = "idle_task",
    .stack_size = 256 * 4,
    .priority = (osPriority_t)osPriorityLow,
};
/* USER CODE BEGIN PV */
// Queues
osMessageQueueId_t rx_mq;    // byte queue
osMessageQueueId_t cmd_mq;   // line queue
osMessageQueueId_t servo_mq; // target queue (len=1)

// UART DMA RX buffer
static uint8_t rx_dma_buf[RX_BUF_LEN];

// 초기 각도(요청 기준: 팬 0°, 틸트 90°)
static int pan_deg = 0;   // TIM2_CH1 (PA0)
static int tilt_deg = 90; // TIM2_CH2 (PA1)

// idle
osEventFlagsId_t ev_idle;
static volatile uint16_t idle_10ms = 0;
static volatile uint8_t reset_posted = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
void start_servo_task(void *argument);
void start_rx_task(void *argument);
void start_cmd_task(void *argument);
void start_idle_task(void *argument);

/* USER CODE BEGIN PFP */
static void servo_set_pan(int deg);
static void servo_set_tilt(int deg);
static void apply_area_move(char area, int level);
static inline int level_to_step(int l);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static inline int us_to_ccr(int us) { return us; } // TIM2 tick = 1 MHz
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
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  // PWM start and init position
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
  servo_set_pan(pan_deg);
  servo_set_tilt(tilt_deg);

  // USART2 IRQ for IDLE detection
  HAL_NVIC_SetPriority(USART2_IRQn, 6, 0);
  HAL_NVIC_EnableIRQ(USART2_IRQn);

  // Start UART DMA + IDLE reception
  HAL_UARTEx_ReceiveToIdle_DMA(&huart2, rx_dma_buf, RX_BUF_LEN);
  __HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT);

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
  rx_mq = osMessageQueueNew(RX_BUF_LEN, sizeof(uint8_t), NULL);
  cmd_mq = osMessageQueueNew(8, sizeof(cmd_msg_t), NULL);
  servo_mq = osMessageQueueNew(1, sizeof(servo_cmd_t), NULL);

  ev_idle = osEventFlagsNew(NULL);
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of servo_task */
  servo_taskHandle = osThreadNew(start_servo_task, NULL, &servo_task_attributes);

  /* creation of rx_task */
  rx_taskHandle = osThreadNew(start_rx_task, NULL, &rx_task_attributes);

  /* creation of cmd_task */
  cmd_taskHandle = osThreadNew(start_cmd_task, NULL, &cmd_task_attributes);

  /* creation of idle_task */
  idle_taskHandle = osThreadNew(start_idle_task, NULL, &idle_task_attributes);

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
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
 * @brief TIM2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 84 - 1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 20000 - 1;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);
}

/**
 * @brief TIM3 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 8400 - 1;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 100 - 1;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_OC_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_TIMING;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_OC_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
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
 * Enable DMA controller clock
 */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);
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
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance != TIM3)
    return;
  osEventFlagsSet(ev_idle, EV_TICK); // 10ms 틱 알림만
}

// USART2 IDLE+DMA callback → push bytes to rx_mq
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
  if (huart->Instance != USART2)
    return;

  osEventFlagsSet(ev_idle, EV_RX); // 수신 이벤트 발생

  for (uint16_t i = 0; i < Size; i++)
  {
    uint8_t b = rx_dma_buf[i];
    (void)osMessageQueuePut(rx_mq, &b, 0, 0); // ISR-safe, timeout 0
  }

  HAL_UARTEx_ReceiveToIdle_DMA(&huart2, rx_dma_buf, RX_BUF_LEN);
  __HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT);
}

/* angle → pulse and write CCR (요청 기준 반영) */
// PAN: 0..90° → 0.5..1.5ms
static void servo_set_pan(int deg)
{
  if (deg < PAN_MIN_DEG)
    deg = PAN_MIN_DEG;
  if (deg > PAN_MAX_DEG)
    deg = PAN_MAX_DEG;
  int ccr = 500 + (2000 * deg) / SERVO_FULL_DEG; // 500..1500us
  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, us_to_ccr(ccr));
  pan_deg = deg;
}

// TILT: 0..180° → 0.5..2.5ms
static void servo_set_tilt(int deg)
{
  if (deg < TILT_MIN_DEG)
    deg = TILT_MIN_DEG;
  if (deg > TILT_MAX_DEG)
    deg = TILT_MAX_DEG;
  int ccr = 500 + (2000 * deg) / SERVO_FULL_DEG; // 500..2500us
  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, us_to_ccr(ccr));
  tilt_deg = deg;
}

static inline int level_to_step(int l)
{
  return (l == 1) ? 30 : (l == 2) ? 20
                                  : 10; // L1/L2/L3
}

static void apply_area_move(char area, int level)
{
  int dx=0, dy=0, step = level_to_step(level);

  // X축(좌·중·우)
  switch (area) {
    case 'Q': case 'A': case 'Z': dx = -1; break;
    case 'W': case 'S': case 'X': dx =  0; break;
    case 'E': case 'D': case 'C': dx = +1; break;
    default: return;
  }
  // Y축(상·중·하)
  switch (area) {
    case 'Q': case 'W': case 'E': dy = -1; break;
    case 'A': case 'S': case 'D': dy =  0; break;
    case 'Z': case 'X': case 'C': dy = +1; break;
  }
  if (area == 'S') return;

  // 기대 동작: D→오른(+X), A→왼(-X), W→위(-Y), X→아래(+Y)
  int pan_delta  = dy;   // PAN = X
  int tilt_delta = dx;   // TILT = Y
  // 필요 시 아래 부호만 뒤집어 보정
  pan_delta  = -pan_delta;
  tilt_delta = -tilt_delta;

  servo_cmd_t t = {
    .pan  = pan_deg  + pan_delta  * step,
    .tilt = tilt_deg + tilt_delta * step
  };
  if (osMessageQueuePut(servo_mq, &t, 0, 0) != osOK) {
    osMessageQueueReset(servo_mq);
    (void)osMessageQueuePut(servo_mq, &t, 0, 0);
  }
}


/* USER CODE END 4 */

/* USER CODE BEGIN Header_start_servo_task */
/**
 * @brief  Function implementing the servo_task thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_start_servo_task */
void start_servo_task(void *argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
  const uint32_t PERIOD_MS = 20; // 50Hz
  const int RAMP_DEG = 2;

  servo_cmd_t target = {.pan = pan_deg, .tilt = tilt_deg}, tmp;

  for (;;)
  {
    while (osMessageQueueGet(servo_mq, &tmp, NULL, 0) == osOK)
      target = tmp;

    int dp = target.pan - pan_deg;
    if (dp > RAMP_DEG)
      dp = RAMP_DEG;
    if (dp < -RAMP_DEG)
      dp = -RAMP_DEG;
    if (dp)
      servo_set_pan(pan_deg + dp);

    int dt = target.tilt - tilt_deg;
    if (dt > RAMP_DEG)
      dt = RAMP_DEG;
    if (dt < -RAMP_DEG)
      dt = -RAMP_DEG;
    if (dt)
      servo_set_tilt(tilt_deg + dt);

    osDelay(PERIOD_MS);
  }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_start_rx_task */
/**
 * @brief Function implementing the rx_task thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_start_rx_task */
void start_rx_task(void *argument)
{
  /* USER CODE BEGIN start_rx_task */
  /* Infinite loop */
  cmd_msg_t msg;
  size_t len = 0;
  uint8_t b;
  for (;;)
  {
    osMessageQueueGet(rx_mq, &b, NULL, osWaitForever);
    if (b == '\r')
      continue;
    if (b == '\n')
    {
      if (len)
      {
        msg.line[len] = 0;
        osMessageQueuePut(cmd_mq, &msg, 0, 0);
        len = 0;
      }
    }
    else
    {
      if (len < sizeof msg.line - 1)
        msg.line[len++] = (char)b;
      else
        len = 0;
    }
  }
  /* USER CODE END start_rx_task */
}

/* USER CODE BEGIN Header_start_cmd_task */
/**
 * @brief Function implementing the cmd_task thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_start_cmd_task */
void start_cmd_task(void *argument)
{
  /* USER CODE BEGIN start_cmd_task */
  /* Infinite loop */
  cmd_msg_t m;
  for (;;)
  {
    if (osMessageQueueGet(cmd_mq, &m, NULL, osWaitForever) != osOK)
      continue;

    if (m.line[0] == 'R' && m.line[1] == 0)
    {
      servo_cmd_t t = {.pan = 0, .tilt = 90};
      osMessageQueueReset(servo_mq);
      osMessageQueuePut(servo_mq, &t, 0, 0);
    }
    else if (m.line[1] == '@' && (m.line[2] >= '1' && m.line[2] <= '3'))
    {
      apply_area_move(m.line[0], m.line[2] - '0');
    }
  }
  /* USER CODE END start_cmd_task */
}

/* USER CODE BEGIN Header_start_idle_task */
/**
 * @brief Function implementing the idle_task thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_start_idle_task */
void start_idle_task(void *argument)
{
  /* USER CODE BEGIN start_idle_task */
  /* Infinite loop */
  // Start 10ms ticktimer
  HAL_TIM_Base_Start_IT(&htim3);
  for (;;)
  {
    uint32_t f = osEventFlagsWait(ev_idle, EV_TICK | EV_RX, osFlagsWaitAny, osWaitForever);

    if (f & EV_RX)
    { // 수신 발생 시 타임아웃 타이머 리셋
      idle_10ms = 0;
      reset_posted = 0;
    }

    if (f & EV_TICK)
    { // 10ms 틱 누적
      if (++idle_10ms >= 300)
      { // 3초
        idle_10ms = 300;
        if (!reset_posted)
        {
          servo_cmd_t t = {.pan = 0, .tilt = 90};
          osMessageQueueReset(servo_mq);
          (void)osMessageQueuePut(servo_mq, &t, 0, 0);
          reset_posted = 1;
        }
      }
    }
  }
  /* USER CODE END start_idle_task */
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
#ifdef USE_FULL_ASSERT
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
