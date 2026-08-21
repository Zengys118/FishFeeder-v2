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
#include "i2c.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stepper.h"
#include "ds3231.h"
#include <stdio.h>
#include <stdarg.h>
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

/* USER CODE BEGIN PV */


/* ================= 调试 ================= */

#define DEBUG_PRINT_ENABLE          1
/*
 * 步进电机开机调试
 *
 * 1 = 每次开机 / Reset 后，自动运行 FEED_STEPS 步
 * 0 = 关闭
 */
#define STEPPER_DEBUG_ENABLE        0

/* ================= 步进电机 ================= */

#define FEED_STEPS                  15000
#define STEPPER_DELAY_MS            2
#define FEED_DIRECTION              1


/* ================= DS3231 校时 ================= */

/*
 * 需要校时时改成 1
 * 校准完成后一定改回 0
 */
#define RTC_SET_TIME_ON_BOOT        0

#define RTC_SET_YEAR                2026
#define RTC_SET_MONTH               8
#define RTC_SET_DATE                20

#define RTC_SET_HOUR                13
#define RTC_SET_MINUTE              29
#define RTC_SET_SECOND              0


/* ================= 第一顿 ================= */

#define FEED1_ENABLE                1

#define FEED1_HOUR                  6
#define FEED1_MINUTE                00
#define FEED1_SECOND                0


/* ================= 第二顿 ================= */

#define FEED2_ENABLE                0

#define FEED2_HOUR                  13
#define FEED2_MINUTE                32
#define FEED2_SECOND                0


/* ================= RTC ================= */

#define RTC_CHECK_INTERVAL_MS       100


/* 当前 RTC 时间 */
static DS3231_Time rtc_time;


/* 防止一次触发多次 */
static uint8_t feed1_triggered = 0;
static uint8_t feed2_triggered = 0;


/* 每秒只打印一次 */
static uint8_t last_print_second = 0xFF;


/* RTC 读取失败时，每秒打印一次错误 */
static uint32_t last_rtc_error_tick = 0;


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


#if DEBUG_PRINT_ENABLE == 1

static void Debug_Printf(const char *format, ...)
{
    char buffer[128];
    int len;
    va_list args;

    va_start(args, format);

    len = vsnprintf(
        buffer,
        sizeof(buffer),
        format,
        args
    );

    va_end(args);

    if (len > 0)
    {
        if (len >= sizeof(buffer))
        {
            len = sizeof(buffer) - 1;
        }

        HAL_UART_Transmit(
            &huart1,
            (uint8_t *)buffer,
            (uint16_t)len,
            1000
        );
    }
}

#define DEBUG_PRINT(...) Debug_Printf(__VA_ARGS__)

#else

#define DEBUG_PRINT(...)

#endif


static void Feed(void)
{
    DEBUG_PRINT("FEED START\r\n");


#if FEED_DIRECTION == 1

    Stepper_Forward(
        FEED_STEPS,
        STEPPER_DELAY_MS
    );

#else

    Stepper_Reverse(
        FEED_STEPS,
        STEPPER_DELAY_MS
    );

#endif


    DEBUG_PRINT("FEED END\r\n");
}


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
  MX_I2C1_Init();
  MX_USART1_UART_Init();
/* USER CODE BEGIN 2 */

Stepper_Init();


/* =========================================================
 * 步进电机开机调试
 *
 * 每次 Reset 后只执行一次
 * ========================================================= */
#if STEPPER_DEBUG_ENABLE == 1

DEBUG_PRINT(
    "STEPPER DEBUG START, STEPS = %u\r\n",
    FEED_STEPS
);

#if FEED_DIRECTION == 1

Stepper_Forward(
    FEED_STEPS,
    STEPPER_DELAY_MS
);

#else

Stepper_Reverse(
    FEED_STEPS,
    STEPPER_DELAY_MS
);

#endif

DEBUG_PRINT(
    "STEPPER DEBUG END\r\n"
);

#endif


HAL_Delay(200);

if (DS3231_IsReady() == HAL_OK)
{
    DEBUG_PRINT("DS3231 OK\r\n");

#if RTC_SET_TIME_ON_BOOT == 1

    if (DS3231_SetTime(
            RTC_SET_YEAR,
            RTC_SET_MONTH,
            RTC_SET_DATE,
            RTC_SET_HOUR,
            RTC_SET_MINUTE,
            RTC_SET_SECOND) == HAL_OK)
    {
        DEBUG_PRINT("RTC TIME SET OK\r\n");
    }
    else
    {
        DEBUG_PRINT("RTC TIME SET ERROR\r\n");
    }

#endif
}
else
{
    DEBUG_PRINT("DS3231 NOT FOUND\r\n");
}

/* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
/* USER CODE BEGIN 2 */

DEBUG_PRINT("PROGRAM START\r\n");

Stepper_Init();

HAL_Delay(200);
while (1)
{
    if (DS3231_ReadTime(&rtc_time) == HAL_OK)
    {
#if DEBUG_PRINT_ENABLE == 1

        if (rtc_time.second != last_print_second)
        {
            last_print_second = rtc_time.second;

            DEBUG_PRINT(
                "%04u-%02u-%02u %02u:%02u:%02u\r\n",
                rtc_time.year,
                rtc_time.month,
                rtc_time.date,
                rtc_time.hour,
                rtc_time.minute,
                rtc_time.second
            );
        }

#endif


#if FEED1_ENABLE == 1

        if ((rtc_time.hour   == FEED1_HOUR) &&
            (rtc_time.minute == FEED1_MINUTE) &&
            (rtc_time.second == FEED1_SECOND))
        {
            if (feed1_triggered == 0)
            {
                feed1_triggered = 1;

                DEBUG_PRINT(
                    "FEED1 TRIGGER %02u:%02u:%02u\r\n",
                    rtc_time.hour,
                    rtc_time.minute,
                    rtc_time.second
                );

                Feed();
            }
        }
        else
        {
            feed1_triggered = 0;
        }

#endif


#if FEED2_ENABLE == 1

        if ((rtc_time.hour   == FEED2_HOUR) &&
            (rtc_time.minute == FEED2_MINUTE) &&
            (rtc_time.second == FEED2_SECOND))
        {
            if (feed2_triggered == 0)
            {
                feed2_triggered = 1;

                DEBUG_PRINT(
                    "FEED2 TRIGGER %02u:%02u:%02u\r\n",
                    rtc_time.hour,
                    rtc_time.minute,
                    rtc_time.second
                );

                Feed();
            }
        }
        else
        {
            feed2_triggered = 0;
        }

#endif
    }
    else
    {
#if DEBUG_PRINT_ENABLE == 1

        if (HAL_GetTick() - last_rtc_error_tick >= 1000)
        {
            last_rtc_error_tick = HAL_GetTick();

            DEBUG_PRINT("RTC READ ERROR\r\n");
        }

#endif
    }

    HAL_Delay(RTC_CHECK_INTERVAL_MS);
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

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
