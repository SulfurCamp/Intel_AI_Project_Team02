# 팬틸트 카메라 펌웨어 (STM32F411RE + FreeRTOS)

STM32F411RE로 팬/틸트 서보를 구동하고, UART2(IDLE+DMA)로 라즈베리파이의 명령을 수신해 추적한다. CMSIS-RTOS v2 API만 사용.


## 하드웨어

* 보드: NUCLEO-F411RE
* 서보: MG996R 2개

  * 팬: TIM2 CH1 → **PA0**
  * 틸트: TIM2 CH2 → **PA1**
* 통신: UART2 115200-8-N-1

  * USART2\_RX DMA: **DMA1 Stream5**
  * USART2 글로벌 인터럽트 Enable
* 전원: 서보는 별도 5–7.2 V. **GND 공통**.


## 동작 개요

* 초기 각도: **팬 0°**, **틸트 90°**
* 명령 형식: 3×3 구역 + 거리 레벨

  ```
  Q W E
  A S D
  Z X C
  ```

  * 예) `Q@1`, `W@2`, `C@3` (개행 포함 권장 `\n`)
  * 리셋: `R`
* 각도 매핑

  * 팬 0–90° → **0.5–1.5 ms** (TIM2 1 MHz 기준 CCR=500..1500)
  * 틸트 0–180° → **0.5–2.5 ms** (CCR=500..2500)
* 가감속: 20 ms 주기, **2°/주기**로 추종
* 레벨→이동량: L1=30°, L2=20°, L3=10°


## 소프트웨어 구조

### 타이머 / PWM

* TIM2: PSC=**83**, ARR=**19999** → 1 MHz, 50 Hz PWM
* CH1=PA0, CH2=PA1. PWM 시작 후 CCR로 펄스폭 설정.

### UART2 수신

* `HAL_UARTEx_ReceiveToIdle_DMA()` 사용
* IDLE 콜백에서 수신 바이트를 바이트 큐로 푸시
* Half-transfer 인터럽트 **비활성화** 처리 포함

### FreeRTOS(CMSIS-RTOS v2)

* 큐

  * `rx_mq`: 바이트 큐, 깊이 128
  * `cmd_mq`: 라인 큐, 깊이 8
  * `servo_mq`: 목표각 큐, 깊이 1(최신만 유지)
* 태스크

  * `rx_task` AboveNormal: 바이트→라인 조립
  * `cmd_task` Normal: `Q@N` 파싱→목표각 계산
  * `servo_task` BelowNormal: 가감속으로 PWM 반영
* IRQ 우선순위 권장: USART2=6, DMA1\_Stream5=6
  (FreeRTOS `configMAX_SYSCALL_INTERRUPT_PRIORITY=5` 가정)



## 빌드

### STM32CubeMX 설정 

* RCC: HSI + PLL(84 MHz)
* USART2: 115200, DMA RX 활성, **Global IRQ Enable**
* DMA: DMA1 Stream5 → USART2\_RX
* TIM2: PWM CH1=PA0, CH2=PA1, PSC=83, ARR=19999
* FreeRTOS: CMSIS-RTOS v2, **heap 충분히 증가**(예: heap4, 14 KB+)

### CMake(예시)

```bash
cmake -S . -B build
cmake --build build
```


## 프로토콜

* 라인 종료: `\n` 권장
* 명령:

  * `R` → 팬 0°, 틸트 90°
  * `<AREA>@<LEVEL>`

    * AREA: `Q,W,E,A,S,D,Z,X,C`
    * LEVEL: `1|2|3`
* 처리 규칙:

  * `S`는 정지. 목표각 미변경.
  * `servo_mq` 깊이 1. 최신 목표만 반영.



## 파일/코드 포인트

* `servo_set_pan()`
  `CCR = 500 + 2000 * pan_deg / 180` → 0.5–1.5 ms 사용
* `servo_set_tilt()`
  `CCR = 500 + 2000 * tilt_deg / 180` → 0.5–2.5 ms 사용
* `HAL_UARTEx_RxEventCallback()`
  `osMessageQueuePut(rx_mq, &b, 0, 0);` 후 **IDLE DMA 재시작**



## 테스트 절차

1. 보드 전원, 서보 외부 전원, GND 공통 확인.
2. 펌웨어 플래시 후 시리얼 115200 연결.
3. `R\n` 송신 → 팬 0°, 틸트 90° 확인.
4. 예시 명령:

   * `W@2\n` → 팬 +20°
   * `A@1\n` → 틸트 −30°
   * `C@3\n` → 팬 −10°, 틸트 +10° (현재 각 기준)


## 보정/문제 해결

* 방향 반대:

  * 팬: `apply_area_move()`의 `dx` 부호 반전
  * 틸트: `dy` 부호 반전
* 스트리밍 끊김:

  * USART2/DMA 인터럽트 Enable 확인
  * `HAL_UARTEx_ReceiveToIdle_DMA()` 재호출 경로 확인
