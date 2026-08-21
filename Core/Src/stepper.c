#include "stepper.h"


/*
 * 28BYJ-48 八拍半步驱动表
 *
 * IN1 IN2 IN3 IN4
 *
 * 1   0   0   0
 * 1   1   0   0
 * 0   1   0   0
 * 0   1   1   0
 * 0   0   1   0
 * 0   0   1   1
 * 0   0   0   1
 * 1   0   0   1
 */
static const uint8_t step_sequence[8][4] =
{
    {1, 0, 0, 0},
    {1, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 1, 0},
    {0, 0, 1, 1},
    {0, 0, 0, 1},
    {1, 0, 0, 1}
};


/*
 * 输出八拍表中的其中一拍
 */
static void Stepper_Output(uint8_t index)
{
    HAL_GPIO_WritePin(
        STEP_IN1_GPIO_Port,
        STEP_IN1_Pin,
        step_sequence[index][0] ? GPIO_PIN_SET : GPIO_PIN_RESET
    );

    HAL_GPIO_WritePin(
        STEP_IN2_GPIO_Port,
        STEP_IN2_Pin,
        step_sequence[index][1] ? GPIO_PIN_SET : GPIO_PIN_RESET
    );

    HAL_GPIO_WritePin(
        STEP_IN3_GPIO_Port,
        STEP_IN3_Pin,
        step_sequence[index][2] ? GPIO_PIN_SET : GPIO_PIN_RESET
    );

    HAL_GPIO_WritePin(
        STEP_IN4_GPIO_Port,
        STEP_IN4_Pin,
        step_sequence[index][3] ? GPIO_PIN_SET : GPIO_PIN_RESET
    );
}


/*
 * 初始化
 */
void Stepper_Init(void)
{
    Stepper_Release();
}


/*
 * 关闭所有线圈
 *
 * 防止电机停止后持续发热
 */
void Stepper_Release(void)
{
    HAL_GPIO_WritePin(
        STEP_IN1_GPIO_Port,
        STEP_IN1_Pin,
        GPIO_PIN_RESET
    );

    HAL_GPIO_WritePin(
        STEP_IN2_GPIO_Port,
        STEP_IN2_Pin,
        GPIO_PIN_RESET
    );

    HAL_GPIO_WritePin(
        STEP_IN3_GPIO_Port,
        STEP_IN3_Pin,
        GPIO_PIN_RESET
    );

    HAL_GPIO_WritePin(
        STEP_IN4_GPIO_Port,
        STEP_IN4_Pin,
        GPIO_PIN_RESET
    );
}


/*
 * 正转
 */
void Stepper_Forward(uint32_t steps, uint16_t delay_ms)
{
    uint32_t i;

    for (i = 0; i < steps; i++)
    {
        Stepper_Output(i % 8);

        HAL_Delay(delay_ms);
    }

    Stepper_Release();
}


/*
 * 反转
 */
void Stepper_Reverse(uint32_t steps, uint16_t delay_ms)
{
    uint32_t i;

    for (i = 0; i < steps; i++)
    {
        Stepper_Output(7 - (i % 8));

        HAL_Delay(delay_ms);
    }

    Stepper_Release();
}