#ifndef AHT20_H
#define AHT20_H

#include <main.h>

typedef enum
{
    AHT20_OK, // OK
    AHT20_BUSY, // 设备忙
    AHT20_ACK_ERROR, // ACK回复错误
    AHT20_CRC_ERROR, // CRC校验错误
    AHT20_OTP_ERROR // OTP完整性测试错误
} AHT20_Status_t;

#define AHT20_ADDR 0x38
#define AHT20_TIMEOUT 200

/**
 * @brief 向AHT20发送测量命令
 *
 * @param hi2c I2C句柄指针
 *
 * @retval     AHT20 status
 *
 * @note       需等待80ms测量完毕\n
 *             阻塞式函数
 */
AHT20_Status_t AHT20_Measure(I2C_HandleTypeDef* hi2c);

/**
 * @brief 读取AHT20数据
 *
 * @param hi2c I2C句柄指针
 * @param St   温度指针
 * @param Srh  湿度指针
 *
 * @return AHT20 status
 *
 * @note   返回值/100为实际值\n
 *        阻塞式函数
 */
AHT20_Status_t AHT20_Get_Data(I2C_HandleTypeDef* hi2c, int16_t* St, uint16_t* Srh);

#endif
