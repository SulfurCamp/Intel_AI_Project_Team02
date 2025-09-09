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
#define CMD_BUF_LEN 16  // typedef에서 필요하므로 선 정의
#endif
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct { char line[CMD_BUF_LEN]; } cmd_msg_t;
typedef struct { int pan; int tilt; } servo_cmd_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#ifndef RX_BUF_LEN
#define RX_BUF_LEN 128
#endif

#ifndef CMD_BUF_LEN
#define CMD_BUF_LEN 16
#endif

#define SERVO_MIN_US    500
#define SERVO_MAX_US    2500
#define SERVO_FULL_DEG  180

// 각도 범위(요청 사양에 맞춤)
#define PAN_MIN_DEG      0
#define PAN_MAX_DEG      90     // 팬은 0..90°
#define TILT_MIN_DEG     0
#define TILT_MAX_DEG     180    // 틸트는 0..180°

#define L1_STEP_DEG      30
#define L2_STEP_DEG      20
#define L3_STEP_DEG      10
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
  .priority = (osPriority_t) osPriorityBelowNormal,
};
/* Definitions for rx_task */
osThreadId_t rx_taskHandle;
const osThreadAttr_t rx_task_attributes = {
  .name = "rx_task",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for cmd_task */
osThreadId_t cmd_taskHandle;
const osThreadAttr_t cmd_task_attributes = {
  .name = "cmd_task",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* USER CODE BEGIN PV */
// Queues
osMessageQueueId_t rx_mq;    // byte queue
osMessageQueueId_t cmd_mq;   // line queue
osMessageQueueId_t servo_mq; // target queue (len=1)

// UART DMA RX buffer
static uint8_t rx_dma_buf[RX_BUF_LEN];

// 초기 각도(요청 기준: 팬 0°, 틸트 90°)
static int pan_deg  = 0;    // TIM2_CH1 (PA0)
static int tilt_deg = 90;   // TIM2_CH2 (PA1)
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

/* USER CODE BEGIN PFP */
static void servo_set_pan(int deg);
static void servo_set_tilt(int deg);
static void apply_area_move(char area, int level);
static inline int level_to_step(int l);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static inline int us_to_ccr(int us){ return us; } // TIM2 tick = 1 MHz
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  HAL_Init();
  SystemClock_Config();

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

  osKernelInitialize();

  /* USER CODE BEGIN RTOS_QUEUES */
  rx_mq    = osMessageQueueNew(RX_BUF_LEN, sizeof(uint8_t), NULL);
  cmd_mq   = osMessageQueueNew(8,        sizeof(cmd_msg_t), NULL);
  servo_mq = osMessageQueueNew(1,        sizeof(servo_cmd_t), NULL);
  /* USER CODE END RTOS_QUEUES */

  servo_taskHandle = osThreadNew(start_servo_task, NULL, &servo_task_attributes);
  rx_taskHandle    = osThreadNew(start_rx_task,    NULL, &rx_task_attributes);
  cmd_taskHandle   = osThreadNew(start_cmd_task,   NULL, &cmd_task_attributes);

  osKernelStart();

  while (1) { }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) { Error_Handler(); }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              | RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) { Error_Handler(); }
}

/**
  * @brief TIM2 Initialization Function
  * @retval None
  */
static void MX_TIM2_Init(void)
{
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  htim2.Instance = TIM2;
  htim2.Init.Prescaler         = 84-1;     // 1 MHz
  htim2.Init.CounterMode       = TIM_COUNTERMODE_UP;
  htim2.Init.Period            = 20000-1;  // 20 ms → 50 Hz
  htim2.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK) { Error_Handler(); }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK) { Error_Handler(); }

  sConfigOC.OCMode     = TIM_OCMODE_PWM1;
  sConfigOC.Pulse      = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK) { Error_Handler(); }
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2) != HAL_OK) { Error_Handler(); }

  HAL_TIM_MspPostInit(&htim2);
}

/**
  * @brief TIM3 Initialization Function
  * @retval None
  */
static void MX_TIM3_Init(void)
{
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  htim3.Instance = TIM3;
  htim3.Init.Prescaler         = 84-1;
  htim3.Init.CounterMode       = TIM_COUNTERMODE_UP;
  htim3.Init.Period            = 65535;
  htim3.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_OC_Init(&htim3) != HAL_OK) { Error_Handler(); }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK) { Error_Handler(); }

  sConfigOC.OCMode     = TIM_OCMODE_TIMING;
  sConfigOC.Pulse      = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_OC_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK) { Error_Handler(); }
}

/**
  * @brief USART2 Initialization Function
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{
  huart2.Instance        = USART2;
  huart2.Init.BaudRate   = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits   = UART_STOPBITS_1;
  huart2.Init.Parity     = UART_PARITY_NONE;
  huart2.Init.Mode       = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK) { Error_Handler(); }
}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 6, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);
}

/**
  * @brief GPIO Initialization Function
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin  = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin   = LD2_Pin;
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);
}

/* USER CODE BEGIN 4 */
// USART2 IDLE+DMA callback → push bytes to rx_mq
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
  if (huart->Instance != USART2) return;

  for (uint16_t i = 0; i < Size; i++) {
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
  if (deg < PAN_MIN_DEG) deg = PAN_MIN_DEG;
  if (deg > PAN_MAX_DEG) deg = PAN_MAX_DEG;
  int ccr = 500 + (2000 * deg) / SERVO_FULL_DEG;   // 500..1500us
  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, us_to_ccr(ccr));
  pan_deg = deg;
}

// TILT: 0..180° → 0.5..2.5ms
static void servo_set_tilt(int deg)
{
  if (deg < TILT_MIN_DEG) deg = TILT_MIN_DEG;
  if (deg > TILT_MAX_DEG) deg = TILT_MAX_DEG;
  int ccr = 500 + (2000 * deg) / SERVO_FULL_DEG;   // 500..2500us
  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, us_to_ccr(ccr));
  tilt_deg = deg;
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
  const uint32_t PERIOD_MS = 20; // 50 Hz
  const int RAMP_DEG = 2;        // step per 20 ms

  servo_cmd_t target = (servo_cmd_t){ .pan = pan_deg, .tilt = tilt_deg };
  servo_cmd_t temp;

  for (;;)
  {
    while (osMessageQueueGet(servo_mq, &temp, NULL, 0) == osOK) target = temp;

    int dp = target.pan  - pan_deg;
    if (dp >  RAMP_DEG) dp =  RAMP_DEG;
    if (dp < -RAMP_DEG) dp = -RAMP_DEG;
    if (dp) servo_set_pan(pan_deg + dp);

    int dt = target.tilt - tilt_deg;
    if (dt >  RAMP_DEG) dt =  RAMP_DEG;
    if (dt < -RAMP_DEG) dt = -RAMP_DEG;
    if (dt) servo_set_tilt(tilt_deg + dt);

    osDelay(PERIOD_MS);
  }
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
  cmd_msg_t msg; size_t len = 0; uint8_t b;

  for (;;)
  {
    osMessageQueueGet(rx_mq, &b, NULL, osWaitForever);
    if (b == '\r') continue;
    if (b == '\n') {
      if (len) { msg.line[len] = 0; (void)osMessageQueuePut(cmd_mq, &msg, 0, 0); len = 0; }
    } else {
      if (len < sizeof msg.line - 1) msg.line[len++] = (char)b;
      else len = 0;
    }
  }
}

/* USER CODE BEGIN Header_start_cmd_task */
/**
 * @brief Function implementing the cmd_task thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_start_cmd_task */

static inline int level_to_step(int l)
{
  return (l==1)?L1_STEP_DEG : (l==2)?L2_STEP_DEG : L3_STEP_DEG;
}

static void apply_area_move(char area, int level)
{
  int dx=0, dy=0, step = level_to_step(level);

  switch (area) {
    case 'Q': case 'A': case 'Z': dx=-1; break;
    case 'W': case 'S': case 'X': dx= 0; break;
    case 'E': case 'D': case 'C': dx=+1; break;
    default: return;
  }
  switch (area) {
    case 'Q': case 'W': case 'E': dy=-1; break;
    case 'A': case 'S': case 'D': dy= 0; break;
    case 'Z': case 'X': case 'C': dy=+1; break;
  }
  if (area=='S') return;

  servo_cmd_t t = { .pan = pan_deg + dx*step, .tilt = tilt_deg + dy*step };
  if (osMessageQueuePut(servo_mq, &t, 0, 0) != osOK) {
    osMessageQueueReset(servo_mq);
    (void)osMessageQueuePut(servo_mq, &t, 0, 0);
  }
}

void start_cmd_task(void *argument)
{
  cmd_msg_t m;

  for(;;){
    if (osMessageQueueGet(cmd_mq, &m, NULL, osWaitForever) == osOK){
      if (m.line[0]=='R' && m.line[1]==0){
        servo_cmd_t t = { .pan=0, .tilt=90 };   // 리셋: 팬 0°, 틸트 90°
        osMessageQueueReset(servo_mq);
        (void)osMessageQueuePut(servo_mq, &t, 0, 0);
      } else if (m.line[1]=='@' && m.line[2]>='1' && m.line[2]<='3'){
        apply_area_move(m.line[0], m.line[2]-'0');
      }
    }
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  __disable_irq();
  while (1) { }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
  (void)file; (void)line;
}
#endif /* USE_FULL_ASSERT */
