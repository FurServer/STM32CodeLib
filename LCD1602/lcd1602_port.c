/**
 * @file    lcd1602_port.c
 * @brief   LCD1602 移植层
 */

#include "lcd1602_port.h"
#include "i2c.h"
#include "main.h"

// 如果你试图移植到8051且使用SDCC编译器,请在末尾加上 __reentrant
// 然后转到 lcd1602.h:50
static uint8_t lcd1602_port_msg_cb(lcd1602_t *lcd, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    (void)lcd;
    (void)arg_ptr;
    switch (msg) {

    case LCD1602_MSG_INIT: // 需要时在此初始化 GPIO 方向
        HAL_GPIO_WritePin(lcdRW_GPIO_Port, lcdRW_Pin, GPIO_PIN_RESET);
        break;

    case LCD1602_MSG_SET_RS: // arg_int: 0=指令, 1=数据
        HAL_GPIO_WritePin(lcdDC_GPIO_Port, lcdDC_Pin, arg_int ? GPIO_PIN_SET : GPIO_PIN_RESET);
        break;

    case LCD1602_MSG_WRITE_DATA: // arg_int: 数据值 (8-bit: 完整字节; 4-bit: 低四位)
        // uint8_t data = arg_int << 4;
        // HAL_I2C_Master_Transmit(&hi2c1, 0x27 << 1, &data, 1, HAL_MAX_DELAY);
        HAL_I2C_Master_Transmit(&hi2c1, 0x27 << 1, &arg_int, 1, 20);
        break;

    case LCD1602_MSG_PULSE_EN: // E 脉冲: 高 → 低, HD44780 在下降沿锁存数据
        HAL_GPIO_WritePin(lcdE_GPIO_Port, lcdE_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(lcdE_GPIO_Port, lcdE_Pin, GPIO_PIN_RESET);
        break;

    case LCD1602_MSG_DELAY_MS:
        HAL_Delay(arg_int);
        break;

    case LCD1602_MSG_BACKLIGHT: // arg_int = 亮度(0~255)
        break;

    default:
        return 0;
    }

    return 1;
}

void lcd1602_port_init(lcd1602_t *lcd)
{
    lcd->msg_cb = lcd1602_port_msg_cb;
    lcd->mode   = LCD1602_MODE_8BIT;
    lcd1602_init(lcd);
}
