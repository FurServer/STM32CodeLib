/**
 * @file    mpu6050_port.c
 * @brief   MPU6050 移植层（平台相关）—— STM32 HAL / I2C1 实现
 * @note    这是本库唯一与平台相关的文件。移植到其它平台时，
 *          只需替换本文件内三个函数的实现即可，mpu6050.c 无需改动。
 *
 *          这里采用「弱符号默认实现」的方式：mpu6050.c 中的函数指针默认
 *          指向这些 mpu6050_port_* 函数。若你希望运行期动态注入，
 *          也可改为调用 mpu6050_bind()。
 */

#include "mpu6050.h"
#include "i2c.h"

/* I2C 通信超时与重试配置 */
#define MPU6050_I2C_TIMEOUT_MS  100     /* 单次 I2C 操作超时 (ms) */
#define MPU6050_I2C_RETRY        3     /* 失败后最大重试次数       */

/**
 * @brief I2C 读寄存器
 */
uint8_t mpu6050_port_iic_read(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len)
{
    HAL_StatusTypeDef status;
    uint8_t retry;

    for (retry = 0; retry <= MPU6050_I2C_RETRY; retry++) {
        status = HAL_I2C_Mem_Read(&hi2c1, addr, reg, I2C_MEMADD_SIZE_8BIT, buf, len, MPU6050_I2C_TIMEOUT_MS);
        if (status == HAL_OK) {
            return 0;
        }
        /* 重试前复位 I2C 外设，清除潜在的挂起状态 */
        if (retry < MPU6050_I2C_RETRY) {
            HAL_I2C_DeInit(&hi2c1);
            HAL_I2C_Init(&hi2c1);
            HAL_Delay(2);
        }
    }
    return 1;
}

/**
 * @brief I2C 写寄存器
 */
uint8_t mpu6050_port_iic_write(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len)
{
    HAL_StatusTypeDef status;
    uint8_t retry;

    for (retry = 0; retry <= MPU6050_I2C_RETRY; retry++) {
        status = HAL_I2C_Mem_Write(&hi2c1, addr, reg, I2C_MEMADD_SIZE_8BIT, buf, len, MPU6050_I2C_TIMEOUT_MS);
        if (status == HAL_OK) {
            return 0;
        }
        /* 重试前复位 I2C 外设，清除潜在的挂起状态 */
        if (retry < MPU6050_I2C_RETRY) {
            HAL_I2C_DeInit(&hi2c1);
            HAL_I2C_Init(&hi2c1);
            HAL_Delay(2);
        }
    }
    return 1;
}

/**
 * @brief 毫秒延时
 */
void mpu6050_port_delay_ms(uint32_t ms)
{
    HAL_Delay(ms);
}
