#include "aht20.h"

// 校验类型: CRC8
// 多项式：X8+X5+X4+1
// Poly: 0011 0001 0x31
uint8_t aht20_check_crc(const uint8_t* arr)
{
    uint8_t crc = 0xFF;
    for (uint8_t i = 0; i < 6; i++)
    {
        crc ^= arr[i];
        for (uint8_t j = 8; j != 0; j--)
        {
            if (crc & 0x80)
                crc = (crc << 1) ^ 0x31;
            else
                crc = (crc << 1);
        }
    }
    return crc;
}

AHT20_Status_t AHT20_Measure(I2C_HandleTypeDef* hi2c)
{
    uint8_t cmd[3] = {0xAC, 0x33, 0x00}; // 这坨是让AHT20测温度的命令
    const HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(hi2c, AHT20_ADDR << 1, cmd, 3, AHT20_TIMEOUT);
    if (status == HAL_ERROR) return AHT20_ACK_ERROR;
    return AHT20_OK;
}

AHT20_Status_t AHT20_Get_Data(I2C_HandleTypeDef* hi2c, int16_t* St, uint16_t* Srh)
{
    uint8_t data[7] = {0};
    uint32_t temp = 0;
    // 读取数据
    const HAL_StatusTypeDef status = HAL_I2C_Master_Receive(hi2c, AHT20_ADDR << 1, data, 7, AHT20_TIMEOUT);
    if (status == HAL_ERROR) return AHT20_ACK_ERROR;
    // 校验
    if (data[6] != aht20_check_crc(data)) return AHT20_CRC_ERROR;
    if (data[0] & 0x80) return AHT20_BUSY;
    if (!(data[0] & 0x10)) return AHT20_OTP_ERROR;
    // 解算
    temp = (data[1] << 16 | data[2] << 8 | data[3]) >> 4;
    *Srh = (temp * 625) >> 16;
    temp = (data[3] << 16 | data[4] << 8 | data[5]) & 0x000FFFFF;
    *St = (int16_t)(((temp * 1250) >> 16) - 5000);
    return AHT20_OK;
}
