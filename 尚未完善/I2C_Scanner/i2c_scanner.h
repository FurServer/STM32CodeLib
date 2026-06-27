#ifndef I2C_SCANNER_H
#define I2C_SCANNER_H

#include "main.h"

// 设备信息结构体
typedef struct
{
    uint8_t address;
    const char* name;
    const char* description;
} I2C_DeviceInfo_t;

// 扫描结果结构体
typedef struct
{
    uint8_t addresses[128]; // 存储检测到的设备地址
    uint8_t count; // 检测到的设备数量
} I2C_ScanResult_t;

#define I2C_SCANN_TRIALS 1 // 扫描尝试次数
#define I2C_SCANN_TIMEOUT 10 // 扫描超时时间(ms)
// #define I2C_SCANN_DELAY 1 // 扫描间隔时间(ms), 注释以取消延时

// 这几坨纯阻塞操作↓

/**
 * @brief 扫描I2C总线上的所有设备
 *
 * @param hi2c I2C句柄指针
 * @param result 扫描结果结构体指针
 *
 * @retval  none
 *
 * @note    记得初始化结构体:\n
 *          I2C_ScanResult_t result = {0};\n
 *          阻塞式函数
 */
void I2C_Scan_Device(I2C_HandleTypeDef* hi2c, I2C_ScanResult_t* result);

/**
 * @brief 将I2C总线设备数扫描结果打印到串口
 *
 * @param result 扫描结果结构体指针
 * @param huart 串口句柄指针
 * @param timeout 串口传输超时时间(ms)
 *
 * @retval  none
 *
 * @note    阻塞式函数
 */
void I2C_Scan_Print_UART(I2C_ScanResult_t* result, UART_HandleTypeDef* huart, uint32_t timeout);

/**
 * @brief 清空I2C总线设备数扫描结果
 *
 * @param result 扫描结果结构体指针
 *
 * @retval  none
 *
 * @note    阻塞式函数
 */
void I2C_Scan_ClearResult(I2C_ScanResult_t* result);

#endif
