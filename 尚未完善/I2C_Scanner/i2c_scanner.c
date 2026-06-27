#include "i2c_scanner.h"

#include <stdio.h>
#include <string.h>

static const I2C_DeviceInfo_t known_devices[] = {
    {0x27, "PCF8574", "串转并"},
    {0x2C, "QMC5883", "三轴磁力计"},
    {0x38, "AHT20", "温湿度计"},
    {0x3C, "SSD1306/SSD1315", "OLED 128x64"},
    {0x40, "INA226", "电流传感器"},
    {0x68, "MPU6050", "惯性测量单元"},
    {0x76, "BMP280", "气压计"}
};
static const uint16_t known_count = sizeof(known_devices) / sizeof(known_devices[0]);


static const I2C_DeviceInfo_t* i2cAddressQueryDevices(uint8_t address)
{
    for (uint16_t i = 0; i < known_count; i++)
    {
        if (known_devices[i].address == address)
        {
            return &known_devices[i];
        }
    }
    return NULL;
}

void I2C_Scan_Device(I2C_HandleTypeDef* hi2c, I2C_ScanResult_t* result)
{
    // 清空结果
    result->count = 0;

    for (uint8_t addr = 1; addr < 128; addr++)
    {
        HAL_StatusTypeDef status =
            HAL_I2C_IsDeviceReady(hi2c, (addr << 1), I2C_SCANN_TRIALS, I2C_SCANN_TIMEOUT);
        if (status == HAL_OK)
        {
            result->addresses[result->count] = addr;
            result->count++;
        }
#ifdef I2C_SCANN_DELAY
        HAL_Delay(2);
#endif
    }
}

void I2C_Scan_Print_UART(I2C_ScanResult_t* result, UART_HandleTypeDef* huart, uint32_t timeout)
{
    uint8_t buffer[256];
    uint8_t len;

    // 如果未扫描到设备
    if (result->count == 0)
    {
        len = sprintf((char*)buffer, "\033[91mNo I2C device.\033[0m\r\n");
        HAL_UART_Transmit(huart, buffer, len, timeout);
        return;
    }

    len = sprintf((char*)buffer, "\033[42mFound %d I2C device(s):\033[0m\r\n", result->count);
    HAL_UART_Transmit(huart, buffer, len, timeout);

    for (uint8_t i = 0; i < result->count; i++)
    {
        uint8_t addr = result->addresses[i];
        const I2C_DeviceInfo_t* dev = i2cAddressQueryDevices(addr);

        if (dev != NULL)
        {
            len = sprintf((char*)buffer, "[\033[92m%d\033[0m] \033[93m0x%02X\033[0m -> \033[96m%s\033[0m (%s)\r\n", i + 1, addr, dev->name, dev->description);
        }
        else
        {
            len = sprintf((char*)buffer, "[\033[92m%d\033[0m] \033[93m0x%02X\033[0m -> \033[91mUnknown device\033[0m\r\n", i + 1, addr);
        }
        HAL_UART_Transmit(huart, buffer, len, timeout);
    }
}

void I2C_Scan_ClearResult(I2C_ScanResult_t* result)
{
    result->count = 0;
    memset(result->addresses, 0, sizeof(result->addresses));
}
