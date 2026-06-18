#include "switch_i2c1.h"

void Switch_I2C1(uint8_t stat)
{
    if (stat)
    {
        __HAL_RCC_I2C1_CLK_ENABLE();
    }
    else
    {
        __HAL_RCC_I2C1_CLK_DISABLE();
    }
}
