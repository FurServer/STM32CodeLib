/**
* @file    mpu6050.h
 * @brief   精简版 MPU6050 驱动库
 */

#ifndef MPU6050_H
#define MPU6050_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* ------------------------------------------------------------------ */
/* 设备 7 位地址（AD0 接地为 0x68，接高为 0x69）。HAL 需要左移 1 位。 */
#define MPU6050_ADDR_7BIT   0x68
#define MPU6050_I2C_ADDR    (MPU6050_ADDR_7BIT << 1)

/* 加速度量程选项 */
typedef enum {
    MPU6050_ACCEL_2G  = 0x00,
    MPU6050_ACCEL_4G  = 0x08,
    MPU6050_ACCEL_8G  = 0x10,
    MPU6050_ACCEL_16G = 0x18,
} mpu6050_accel_range_t;

/* 陀螺仪量程选项 */
typedef enum {
    MPU6050_GYRO_250DPS  = 0x00,
    MPU6050_GYRO_500DPS  = 0x08,
    MPU6050_GYRO_1000DPS = 0x10,
    MPU6050_GYRO_2000DPS = 0x18,
} mpu6050_gyro_range_t;

/* 运行时数据句柄 */
typedef struct {
    float accel_scale;      /* 加速度 LSB->g 比例     */
    float gyro_scale;       /* 陀螺仪 LSB->dps 比例   */

    float gyro_bias[3];     /* 陀螺仪零偏(dps)，校准得到 */

    float q[4];             /* 四元数 w,x,y,z          */
    float integral_fb[3];   /* Mahony 积分误差         */
    float two_kp;           /* 比例增益 (2*Kp)         */
    float two_ki;           /* 积分增益 (2*Ki)         */
} mpu6050_t;

/* ------------------------------------------------------------------ */

/**
 * @brief  初始化 MPU6050（唤醒、设置量程/采样率/低通滤波），并初始化四元数。
 * @param  dev    设备句柄
 * @param  accel  加速度量程
 * @param  gyro   陀螺仪量程
 * @return 0 成功，非 0 失败
 */
uint8_t mpu6050_init(mpu6050_t *dev, mpu6050_accel_range_t accel, mpu6050_gyro_range_t  gyro);

/**
 * @brief  陀螺仪零偏校准。校准时务必保持模块静止。
 * @param  dev      设备句柄
 * @param  samples  采样次数（建议 1000）
 * @return 0 成功，非 0 失败
 */
uint8_t mpu6050_calibrate_gyro(mpu6050_t *dev, uint16_t samples);

/**
 * @brief  用加速度计当前读数直接计算并设置初始姿态四元数。
 *         适用于上电时传感器处于任意倾斜静止状态，可避免姿态从“水平”
 *         缓慢收敛导致的开机初期数据错误。yaw 无法由加速度计确定，置 0。
 *         mpu6050_init 内部已自动调用一次，通常无需手动调用。
 * @param  dev      设备句柄
 * @param  samples  采样平均次数（建议 16~64，用于抑制噪声）
 * @return 0 成功，非 0 失败
 */
uint8_t mpu6050_set_quaternion_from_accel(mpu6050_t *dev, uint16_t samples);

/**
 * @brief  读取原始 16 位寄存器数据。
 * @param  accel  输出加速度原始值 [x,y,z]
 * @param  gyro   输出陀螺仪原始值 [x,y,z]
 * @return 0 成功，非 0 失败
 */
uint8_t mpu6050_read_raw(int16_t accel[3], int16_t gyro[3]);

/**
 * @brief  读取物理量。加速度单位 g，角速度单位 dps（已扣除零偏）。
 * @return 0 成功，非 0 失败
 */
uint8_t mpu6050_read(mpu6050_t *dev, float accel_g[3], float gyro_dps[3]);

/**
 * @brief  使用 Mahony 互补滤波更新姿态四元数。
 * @param  dev  设备句柄
 * @param  dt   两次调用间隔(秒)
 * @return 0 成功，非 0 失败
 */
uint8_t mpu6050_update(mpu6050_t *dev, float dt);

/**
 * @brief  获取当前四元数。
 */
void mpu6050_get_quaternion(const mpu6050_t *dev, float q[4]);

/**
 * @brief  由四元数计算欧拉角（单位：度）。
 */
void mpu6050_get_euler(const mpu6050_t *dev, float *roll, float *pitch, float *yaw);

/**
 * @brief  读取温度（单位：摄氏度）。
 * @return 0 成功，非 0 失败
 */
uint8_t mpu6050_read_temperature(float *celsius);

/* ================================================================== */
/* 移植接口（Porting Layer）                                           */
/* 为保证库的可移植性，库内部不直接调用任何平台 API。                   */
/* 移植到新平台时，只需在一个单独的 .c 文件中实现下面三个函数即可，      */
/* 库本身（mpu6050.c）无需任何修改。                                    */
/*                                                                     */
/* 也可以在 mpu6050_bind() 中注入函数指针来实现，二选一。               */
/* ================================================================== */

/** I2C 读写函数指针类型：addr 为 8 位 HAL 设备地址，reg 寄存器，buf/len 数据。返回 0 成功。 */
typedef uint8_t (*mpu6050_iic_rw_t)(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len);
/** 毫秒延时函数指针类型 */
typedef void    (*mpu6050_delay_ms_t)(uint32_t ms);

/**
 * @brief  绑定平台相关的底层接口（运行期注入，便于多平台/单元测试）。
 *         若未调用本函数，库会回退到弱符号默认实现 mpu6050_port_*。
 * @param  read   I2C 读接口
 * @param  write  I2C 写接口
 * @param  delay  毫秒延时接口
 */
void mpu6050_bind(mpu6050_iic_rw_t read, mpu6050_iic_rw_t write, mpu6050_delay_ms_t delay);

/* 以下为默认弱符号接口，用户可在自己的 port 文件中直接实现（覆盖弱符号），
 * 即可无需调用 mpu6050_bind 也能工作。 */
uint8_t mpu6050_port_iic_read (uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len);
uint8_t mpu6050_port_iic_write(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len);
void    mpu6050_port_delay_ms (uint32_t ms);

#ifdef __cplusplus
}
#endif

#endif /* MPU6050_H */
