#include "u8g2_stm32.h"

uint8_t u8x8_byte_stm32_hw_i2c(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr)
{
    /* u8g2/u8x8 will never send more than 32 bytes between START_TRANSFER and END_TRANSFER */
    static uint8_t buffer[128];
    static uint8_t buf_idx;
    uint8_t* data;

    switch (msg)
    {
    case U8X8_MSG_BYTE_INIT:
        break;

    case U8X8_MSG_BYTE_START_TRANSFER:
        {
            buf_idx = 0;
        }
        break;

    case U8X8_MSG_BYTE_SEND:
        {
            data = (uint8_t*)arg_ptr;

            while (arg_int > 0)
            {
                buffer[buf_idx++] = *data;
                data++;
                arg_int--;
            }
        }
        break;

    case U8X8_MSG_BYTE_END_TRANSFER:
        {
            if (HAL_I2C_Master_Transmit(&hi2c1, 0x3C << 1, buffer, buf_idx, 100) != HAL_OK)
                return 0;
        }
        break;

    default:
        return 0;
    }

    return 1;
}

uint8_t u8x8_byte_stm32_hw_spi(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr)
{
    switch (msg)
    {
    case U8X8_MSG_BYTE_SEND:
        HAL_SPI_Transmit(&hspi1, arg_ptr, arg_int, HAL_MAX_DELAY);
        break;

    case U8X8_MSG_BYTE_INIT:
        break;

    case U8X8_MSG_BYTE_SET_DC:
        HAL_GPIO_WritePin(lcdDC_GPIO_Port, lcdDC_Pin, (GPIO_PinState)arg_int);
        break;

    case U8X8_MSG_BYTE_START_TRANSFER:
        CS_Select(2);
        break;

    case U8X8_MSG_BYTE_END_TRANSFER:
        CS_Deselect();
        break;

    default:
        return 0;
    }
    return 1;
}


uint8_t u8x8_stm32_gpio_and_delay(U8X8_UNUSED u8x8_t* u8x8, U8X8_UNUSED uint8_t msg, U8X8_UNUSED uint8_t arg_int, U8X8_UNUSED void* arg_ptr)
{
    switch (msg)
    {
    case U8X8_MSG_GPIO_AND_DELAY_INIT:
        break;
    case U8X8_MSG_DELAY_MILLI:
        HAL_Delay(arg_int);
        break;
    case U8X8_MSG_GPIO_RESET:
        break;
    default:
        return 0;
    }
    return 1;
}

/********************************************
U8G2_R0     //不旋转，不镜像
U8G2_R1     //旋转90度
U8G2_R2     //旋转180度
U8G2_R3     //旋转270度
U8G2_MIRROR   //没有旋转，横向显示左右镜像
U8G2_MIRROR_VERTICAL    //没有旋转，竖向显示镜像
********************************************/
void u8g2Init_SPI(u8g2_t* u8g2)
{
    u8g2_Setup_ssd1315_128x64_noname_f(
        u8g2,
        U8G2_R2,
        u8x8_byte_stm32_hw_spi,
        u8x8_stm32_gpio_and_delay
    );
    u8g2_InitDisplay(u8g2);
    u8g2_SetPowerSave(u8g2, 0);
}

void u8g2Init_I2C(u8g2_t* u8g2)
{
    u8g2_Setup_ssd1315_i2c_128x64_noname_f(
        u8g2,
        U8G2_R0,
        u8x8_byte_stm32_hw_i2c,
        u8x8_stm32_gpio_and_delay
    );
    u8g2_InitDisplay(u8g2);
    u8g2_SetPowerSave(u8g2, 0);
}

void draw_u8g2_example(u8g2_t* u8g2)
{
    u8g2_SetFontMode(u8g2, 1); /*字体模式选择*/
    u8g2_SetFontDirection(u8g2, 0); /*字体方向选择*/
    u8g2_SetFont(u8g2, u8g2_font_inb24_mf); /*字库选择*/
    u8g2_DrawStr(u8g2, 0, 20, "U");

    u8g2_SetFontDirection(u8g2, 1);
    u8g2_SetFont(u8g2, u8g2_font_inb30_mn);
    u8g2_DrawStr(u8g2, 21, 8, "8");

    u8g2_SetFontDirection(u8g2, 0);
    u8g2_SetFont(u8g2, u8g2_font_inb24_mf);
    u8g2_DrawStr(u8g2, 51, 30, "g");
    u8g2_DrawStr(u8g2, 67, 30, "\xb2");

    u8g2_DrawHLine(u8g2, 2, 35, 47);
    u8g2_DrawHLine(u8g2, 3, 36, 47);
    u8g2_DrawVLine(u8g2, 45, 32, 12);
    u8g2_DrawVLine(u8g2, 46, 33, 12);

    u8g2_SetFont(u8g2, u8g2_font_4x6_tr);
    u8g2_DrawStr(u8g2, 1, 54, "github.com/olikraus/u8g2");
}
