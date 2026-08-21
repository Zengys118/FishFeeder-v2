#include "ds3231.h"
#include "i2c.h"


/*
 * DS3231 7-bit 地址 = 0x68
 *
 * STM32 HAL 使用时左移一位
 */
#define DS3231_ADDRESS    (0x68 << 1)


/*
 * 十进制 -> BCD
 */
static uint8_t DecToBCD(uint8_t value)
{
    return ((value / 10) << 4) |
           (value % 10);
}


/*
 * BCD -> 十进制
 */
static uint8_t BCDToDec(uint8_t value)
{
    return ((value >> 4) * 10) +
           (value & 0x0F);
}


/*
 * 根据日期计算星期
 *
 * 返回：
 * 1 = Monday
 * 2 = Tuesday
 * ...
 * 7 = Sunday
 */
static uint8_t CalculateDayOfWeek(
    uint16_t year,
    uint8_t month,
    uint8_t date
)
{
    uint16_t y;
    uint8_t m;
    uint8_t day;

    y = year;
    m = month;

    if (m < 3)
    {
        m += 12;
        y--;
    }

    day =
        (
            date +
            (13 * (m + 1)) / 5 +
            (y % 100) +
            (y % 100) / 4 +
            (y / 100) / 4 +
            5 * (y / 100)
        ) % 7;

    if (day == 0)
    {
        return 6;
    }
    else if (day == 1)
    {
        return 7;
    }

    return day - 1;
}


/*
 * 检查 DS3231 是否存在
 */
HAL_StatusTypeDef DS3231_IsReady(void)
{
    return HAL_I2C_IsDeviceReady(
        &hi2c1,
        DS3231_ADDRESS,
        3,
        100
    );
}


/*
 * 设置 DS3231 时间
 */
HAL_StatusTypeDef DS3231_SetTime(
    uint16_t year,
    uint8_t month,
    uint8_t date,
    uint8_t hour,
    uint8_t minute,
    uint8_t second
)
{
    uint8_t data[7];

    data[0] = DecToBCD(second);
    data[1] = DecToBCD(minute);
    data[2] = DecToBCD(hour);

    data[3] =
        DecToBCD(
            CalculateDayOfWeek(
                year,
                month,
                date
            )
        );

    data[4] = DecToBCD(date);
    data[5] = DecToBCD(month);

    data[6] =
        DecToBCD(
            (uint8_t)(year - 2000)
        );

    return HAL_I2C_Mem_Write(
        &hi2c1,
        DS3231_ADDRESS,
        0x00,
        I2C_MEMADD_SIZE_8BIT,
        data,
        7,
        100
    );
}


/*
 * 读取 DS3231 时间
 */
HAL_StatusTypeDef DS3231_ReadTime(
    DS3231_Time *time
)
{
    uint8_t data[7];

    HAL_StatusTypeDef status;

    status = HAL_I2C_Mem_Read(
        &hi2c1,
        DS3231_ADDRESS,
        0x00,
        I2C_MEMADD_SIZE_8BIT,
        data,
        7,
        100
    );

    if (status != HAL_OK)
    {
        return status;
    }


    time->second =
        BCDToDec(data[0] & 0x7F);

    time->minute =
        BCDToDec(data[1] & 0x7F);


    /*
     * 小时寄存器
     *
     * 兼容 12/24 小时模式
     */
    if (data[2] & 0x40)
    {
        uint8_t hour;

        hour =
            BCDToDec(
                data[2] & 0x1F
            );

        if (data[2] & 0x20)
        {
            if (hour != 12)
            {
                hour += 12;
            }
        }
        else
        {
            if (hour == 12)
            {
                hour = 0;
            }
        }

        time->hour = hour;
    }
    else
    {
        time->hour =
            BCDToDec(
                data[2] & 0x3F
            );
    }


    time->day =
        BCDToDec(
            data[3] & 0x07
        );

    time->date =
        BCDToDec(
            data[4] & 0x3F
        );

    time->month =
        BCDToDec(
            data[5] & 0x1F
        );

    time->year =
        2000 +
        BCDToDec(data[6]);


    return HAL_OK;
}


/*
 * 长期走时微调
 *
 * 目前保持 0 即可
 */
HAL_StatusTypeDef DS3231_SetAgingOffset(
    int8_t offset
)
{
    uint8_t value;

    value = (uint8_t)offset;

    return HAL_I2C_Mem_Write(
        &hi2c1,
        DS3231_ADDRESS,
        0x10,
        I2C_MEMADD_SIZE_8BIT,
        &value,
        1,
        100
    );
}