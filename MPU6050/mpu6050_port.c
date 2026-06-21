/**
 * @file    mpu6050_port.c
 * @brief   MPU6050 移植层
 * @note    这是本库唯一与平台相关的文件。移植到其它平台时，
 *          只需替换本文件内三个函数的实现即可，mpu6050.c 无需改动。
 *
 *          这里采用「弱符号默认实现」的方式：mpu6050.c 中的函数指针默认
 *          指向这些 mpu6050_port_* 函数。若你希望运行期动态注入，
 *          也可改为调用 mpu6050_bind()。
 */

#include "mpu6050.h"
#include "i2c.h"     /* 提供 hi2c1 句柄 */

/**
 * @brief I2C 读寄存器
 */
uint8_t mpu6050_port_iic_read(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len)
{
    if (HAL_I2C_Mem_Read(&hi2c1, addr, reg, I2C_MEMADD_SIZE_8BIT, buf, len, HAL_MAX_DELAY) != HAL_OK) {
        return 1;
    }
    return 0;
}

/**
 * @brief I2C 写寄存器
 */
uint8_t mpu6050_port_iic_write(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len)
{
    if (HAL_I2C_Mem_Write(&hi2c1, addr, reg, I2C_MEMADD_SIZE_8BIT, buf, len, HAL_MAX_DELAY) != HAL_OK) {
        return 1;
    }
    return 0;
}

/**
 * @brief 毫秒延时
 */
void mpu6050_port_delay_ms(uint32_t ms)
{
    HAL_Delay(ms);
}
