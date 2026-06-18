#ifndef SWITCH_I2C1_H
#define SWITCH_I2C1_H

#include <stdint.h>
#include "i2c.h"

extern I2C_HandleTypeDef hi2c1;

typedef enum
{
    I2C1_OFF = 0u,
    I2C1_ON
} I2C1_State;

/**
 * @brief  使能或禁用I2C1

 * @param  stat 状态 I2C1_ON / I2C1_OFF
 *
 * @retval None
 *
 * @note STM32F103系列48pin芯片设计缺陷\n
 *       当使能I2C1时,\n
 *       PB5(SMBA)只能作为GPIO使用\n
 *       受影响的功能: \n
 *       SPI1重映射(MOSI)\n
 *       TIM3CH2重映射\n\n
 *       函数通过使能或禁用I2C1时钟\n
 *       实现与其他功能分时复用\n
 *       __HAL_RCC_I2C1_CLK_ENABLE();\n
 *       __HAL_RCC_I2C1_CLK_DISABLE();
 */
void Switch_I2C1(uint8_t stat);


#endif
