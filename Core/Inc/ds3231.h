#ifndef __DS3231_H
#define __DS3231_H

#include "main.h"
#include <stdint.h>


typedef struct
{
    uint16_t year;

    uint8_t month;
    uint8_t date;
    uint8_t day;

    uint8_t hour;
    uint8_t minute;
    uint8_t second;

} DS3231_Time;


/* 检查 DS3231 是否在线 */
HAL_StatusTypeDef DS3231_IsReady(void);


/* 读取时间 */
HAL_StatusTypeDef DS3231_ReadTime(
    DS3231_Time *time
);


/* 设置时间 */
HAL_StatusTypeDef DS3231_SetTime(
    uint16_t year,
    uint8_t month,
    uint8_t date,
    uint8_t hour,
    uint8_t minute,
    uint8_t second
);


/*
 * DS3231 长期走时微调
 *
 * 现在不用动
 */
HAL_StatusTypeDef DS3231_SetAgingOffset(
    int8_t offset
);


#endif
