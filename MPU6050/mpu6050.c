/**
* @file    mpu6050.c
 * @brief   精简版 MPU6050 驱动实现（基础读取 + 零偏校准 + 四元数解算）
 * @note    本文件不依赖任何具体平台 API，所有底层操作通过移植接口完成，
 *          因此可直接移植到 STM32 / ESP32 / Linux 等任意平台。
 */

#include "mpu6050.h"
#include <math.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/* MPU6050 寄存器地址                                                  */
#define REG_SMPLRT_DIV    0x19
#define REG_CONFIG        0x1A
#define REG_GYRO_CONFIG   0x1B
#define REG_ACCEL_CONFIG  0x1C
#define REG_ACCEL_XOUT_H  0x3B
#define REG_TEMP_OUT_H    0x41
#define REG_GYRO_XOUT_H   0x43
#define REG_PWR_MGMT_1    0x6B
#define REG_WHO_AM_I      0x75

#define WHO_AM_I_VALUE    0x68      // 有时候非正版可能是0x70

/* ------------------------------------------------------------------ */
/* 底层接口函数指针：默认指向弱符号 port 实现，可被 mpu6050_bind 覆盖。 */
static mpu6050_iic_rw_t   g_read  = mpu6050_port_iic_read;
static mpu6050_iic_rw_t   g_write = mpu6050_port_iic_write;
static mpu6050_delay_ms_t g_delay = mpu6050_port_delay_ms;

void mpu6050_bind(mpu6050_iic_rw_t read, mpu6050_iic_rw_t write, mpu6050_delay_ms_t delay)
{
    if (read)  g_read  = read;
    if (write) g_write = write;
    if (delay) g_delay = delay;
}

/* ------------------------------------------------------------------ */
/* 寄存器读写小工具                                                    */
static uint8_t reg_write_byte(uint8_t reg, uint8_t value)
{
    return g_write(MPU6050_I2C_ADDR, reg, &value, 1);
}

static uint8_t reg_read(uint8_t reg, uint8_t *buf, uint16_t len)
{
    return g_read(MPU6050_I2C_ADDR, reg, buf, len);
}

/* 将连续的 6 字节大端数据转换为 3 个 int16 */
static void be16x3(const uint8_t *p, int16_t out[3])
{
    out[0] = (int16_t)((p[0] << 8) | p[1]);
    out[1] = (int16_t)((p[2] << 8) | p[3]);
    out[2] = (int16_t)((p[4] << 8) | p[5]);
}

/* ------------------------------------------------------------------ */
uint8_t mpu6050_init(mpu6050_t *dev, mpu6050_accel_range_t accel, mpu6050_gyro_range_t  gyro)
{
    uint8_t id = 0;

    if (dev == NULL) return 1;

    memset(dev, 0, sizeof(mpu6050_t));
    dev->q[0]   = 1.0f;          /* 单位四元数 */
    dev->two_kp = 2.0f * 0.5f;   /* Mahony 默认增益 */
    dev->two_ki = 2.0f * 0.0f;

    /* 校验设备 ID */
    if (reg_read(REG_WHO_AM_I, &id, 1) != 0) return 1;
    if (id != WHO_AM_I_VALUE)                return 1;

    /* 唤醒：PWR_MGMT_1 = 0x01，选用陀螺仪 X 轴 PLL 作为时钟源 */
    if (reg_write_byte(REG_PWR_MGMT_1, 0x01) != 0) return 1;
    g_delay(50);

    /* 采样率：1kHz / (1 + SMPLRT_DIV)。这里设为 7 -> 125Hz */
    if (reg_write_byte(REG_SMPLRT_DIV, 0x07) != 0) return 1;

    /* 低通滤波 DLPF = 3 (~44Hz) */
    if (reg_write_byte(REG_CONFIG, 0x03) != 0) return 1;

    /* 设置陀螺仪 / 加速度量程 */
    if (reg_write_byte(REG_GYRO_CONFIG,  (uint8_t)gyro)  != 0) return 1;
    if (reg_write_byte(REG_ACCEL_CONFIG, (uint8_t)accel) != 0) return 1;

    /* 计算量程换算比例 */
    switch (accel) {
        case MPU6050_ACCEL_2G:  dev->accel_scale = 1.0f / 16384.0f; break;
        case MPU6050_ACCEL_4G:  dev->accel_scale = 1.0f / 8192.0f;  break;
        case MPU6050_ACCEL_8G:  dev->accel_scale = 1.0f / 4096.0f;  break;
        case MPU6050_ACCEL_16G: dev->accel_scale = 1.0f / 2048.0f;  break;
        default:                dev->accel_scale = 1.0f / 16384.0f; break;
    }
    switch (gyro) {
        case MPU6050_GYRO_250DPS:  dev->gyro_scale = 1.0f / 131.0f;  break;
        case MPU6050_GYRO_500DPS:  dev->gyro_scale = 1.0f / 65.5f;   break;
        case MPU6050_GYRO_1000DPS: dev->gyro_scale = 1.0f / 32.8f;   break;
        case MPU6050_GYRO_2000DPS: dev->gyro_scale = 1.0f / 16.4f;   break;
        default:                   dev->gyro_scale = 1.0f / 131.0f;  break;
    }

    g_delay(50);

    /* 用加速度计直接计算初始姿态，避免上电时从“水平”缓慢收敛导致的初期数据错误 */
    mpu6050_set_quaternion_from_accel(dev, 32);

    return 0;
}

/* ------------------------------------------------------------------ */
uint8_t mpu6050_calibrate_gyro(mpu6050_t *dev, uint16_t samples)
{
    float sum[3] = {0.0f, 0.0f, 0.0f};
    int16_t accel[3], gyro[3];
    uint16_t i;

    if (dev == NULL || samples == 0) return 1;

    /* 校准前先清零旧零偏 */
    dev->gyro_bias[0] = dev->gyro_bias[1] = dev->gyro_bias[2] = 0.0f;

    for (i = 0; i < samples; i++) {
        if (mpu6050_read_raw(accel, gyro) != 0) return 1;
        sum[0] += (float)gyro[0];
        sum[1] += (float)gyro[1];
        sum[2] += (float)gyro[2];
        g_delay(1);
    }

    /* 取平均并换算成 dps 作为零偏 */
    dev->gyro_bias[0] = (sum[0] / samples) * dev->gyro_scale;
    dev->gyro_bias[1] = (sum[1] / samples) * dev->gyro_scale;
    dev->gyro_bias[2] = (sum[2] / samples) * dev->gyro_scale;

    return 0;
}

/* ------------------------------------------------------------------ */
uint8_t mpu6050_read_raw(int16_t accel[3], int16_t gyro[3])
{
    uint8_t buf[14];

    /* 一次性读取 加速度(6) + 温度(2) + 陀螺仪(6) = 14 字节，地址连续 */
    if (reg_read(REG_ACCEL_XOUT_H, buf, 14) != 0) return 1;

    if (accel) be16x3(&buf[0],  accel);
    if (gyro)  be16x3(&buf[8],  gyro);

    return 0;
}

/* ------------------------------------------------------------------ */
uint8_t mpu6050_read(mpu6050_t *dev, float accel_g[3], float gyro_dps[3])
{
    int16_t a[3], g[3];

    if (dev == NULL) return 1;
    if (mpu6050_read_raw(a, g) != 0) return 1;

    if (accel_g) {
        accel_g[0] = a[0] * dev->accel_scale;
        accel_g[1] = a[1] * dev->accel_scale;
        accel_g[2] = a[2] * dev->accel_scale;
    }
    if (gyro_dps) {
        gyro_dps[0] = g[0] * dev->gyro_scale - dev->gyro_bias[0];
        gyro_dps[1] = g[1] * dev->gyro_scale - dev->gyro_bias[1];
        gyro_dps[2] = g[2] * dev->gyro_scale - dev->gyro_bias[2];
    }
    return 0;
}

/* ------------------------------------------------------------------ */
uint8_t mpu6050_read_temperature(float *celsius)
{
    uint8_t buf[2];
    int16_t raw;

    if (celsius == NULL) return 1;
    if (reg_read(REG_TEMP_OUT_H, buf, 2) != 0) return 1;

    raw = (int16_t)((buf[0] << 8) | buf[1]);
    /* 数据手册公式：T = raw/340 + 36.53 */
    *celsius = raw / 340.0f + 36.53f;
    return 0;
}

/* ------------------------------------------------------------------ */
// 快速平方根倒数
static float inv_sqrt(float x)
{
    float halfx = 0.5f * x;
    float y = x;
    union { float f; long l; } u;
    u.f = y;
    u.l = 0x5f3759df - (u.l >> 1);
    y = u.f;
    y = y * (1.5f - (halfx * y * y));
    y = y * (1.5f - (halfx * y * y));
    return y;
}

/* ------------------------------------------------------------------ */
uint8_t mpu6050_set_quaternion_from_accel(mpu6050_t *dev, uint16_t samples)
{
    float sum[3] = {0.0f, 0.0f, 0.0f};
    float ax, ay, az, recip_norm;
    float roll, pitch;
    float cr, sr, cp, sp;
    int16_t a[3], g[3];
    uint16_t i;

    if (dev == NULL) return 1;
    if (samples == 0) samples = 1;

    /* 多次采样取平均，降低加速度计噪声对初始姿态的影响 */
    for (i = 0; i < samples; i++) {
        if (mpu6050_read_raw(a, g) != 0) return 1;
        sum[0] += (float)a[0];
        sum[1] += (float)a[1];
        sum[2] += (float)a[2];
        g_delay(1);
    }

    ax = sum[0] / (float)samples;
    ay = sum[1] / (float)samples;
    az = sum[2] / (float)samples;

    /* 加速度全为 0（异常）时退回单位四元数 */
    if ((ax == 0.0f) && (ay == 0.0f) && (az == 0.0f)) {
        dev->q[0] = 1.0f;
        dev->q[1] = dev->q[2] = dev->q[3] = 0.0f;
        return 1;
    }

    /* 归一化重力方向向量 */
    recip_norm = inv_sqrt(ax * ax + ay * ay + az * az);
    ax *= recip_norm;
    ay *= recip_norm;
    az *= recip_norm;

    /* 由重力方向求 roll / pitch（yaw 无法由加速度计确定，取 0），
     * 约定与 mpu6050_get_euler 一致：roll 绕 X 轴，pitch 绕 Y 轴。 */
    roll  = atan2f(ay, az);
    pitch = atan2f(-ax, sqrtf(ay * ay + az * az));

    /* 欧拉角(ZYX, yaw=0) 转四元数 */
    cr = cosf(roll  * 0.5f);
    sr = sinf(roll  * 0.5f);
    cp = cosf(pitch * 0.5f);
    sp = sinf(pitch * 0.5f);

    dev->q[0] =  cr * cp;
    dev->q[1] =  sr * cp;
    dev->q[2] =  cr * sp;
    dev->q[3] = -sr * sp;

    /* 清空积分项，避免历史误差残留 */
    dev->integral_fb[0] = dev->integral_fb[1] = dev->integral_fb[2] = 0.0f;

    return 0;
}

/* ------------------------------------------------------------------ */
uint8_t mpu6050_update(mpu6050_t *dev, float dt)
{
    float accel_g[3], gyro_dps[3];
    float gx, gy, gz, ax, ay, az;
    float recip_norm;
    float halfvx, halfvy, halfvz;
    float halfex, halfey, halfez;
    float qa, qb, qc;

    if (dev == NULL || dt <= 0.0f) return 1;
    if (mpu6050_read(dev, accel_g, gyro_dps) != 0) return 1;

    /* 角速度由 dps 转换为 rad/s */
    gx = gyro_dps[0] * 0.017453293f;
    gy = gyro_dps[1] * 0.017453293f;
    gz = gyro_dps[2] * 0.017453293f;

    ax = accel_g[0];
    ay = accel_g[1];
    az = accel_g[2];

    /* 仅当加速度有效时才用其做姿态修正（避免除零） */
    if (!((ax == 0.0f) && (ay == 0.0f) && (az == 0.0f))) {
        recip_norm = inv_sqrt(ax * ax + ay * ay + az * az);
        ax *= recip_norm;
        ay *= recip_norm;
        az *= recip_norm;

        /* 由四元数估计的重力方向 */
        halfvx = dev->q[1] * dev->q[3] - dev->q[0] * dev->q[2];
        halfvy = dev->q[0] * dev->q[1] + dev->q[2] * dev->q[3];
        halfvz = dev->q[0] * dev->q[0] - 0.5f + dev->q[3] * dev->q[3];

        /* 测量重力与估计重力的叉积 = 误差 */
        halfex = (ay * halfvz - az * halfvy);
        halfey = (az * halfvx - ax * halfvz);
        halfez = (ax * halfvy - ay * halfvx);

        /* 积分项（Ki 不为 0 时启用） */
        if (dev->two_ki > 0.0f) {
            dev->integral_fb[0] += dev->two_ki * halfex * dt;
            dev->integral_fb[1] += dev->two_ki * halfey * dt;
            dev->integral_fb[2] += dev->two_ki * halfez * dt;
            gx += dev->integral_fb[0];
            gy += dev->integral_fb[1];
            gz += dev->integral_fb[2];
        }

        /* 比例项 */
        gx += dev->two_kp * halfex;
        gy += dev->two_kp * halfey;
        gz += dev->two_kp * halfez;
    }

    /* 一阶积分更新四元数 */
    gx *= 0.5f * dt;
    gy *= 0.5f * dt;
    gz *= 0.5f * dt;
    qa = dev->q[0];
    qb = dev->q[1];
    qc = dev->q[2];
    dev->q[0] += (-qb * gx - qc * gy - dev->q[3] * gz);
    dev->q[1] += ( qa * gx + qc * gz - dev->q[3] * gy);
    dev->q[2] += ( qa * gy - qb * gz + dev->q[3] * gx);
    dev->q[3] += ( qa * gz + qb * gy - qc * gx);

    /* 归一化四元数 */
    recip_norm = inv_sqrt(dev->q[0] * dev->q[0] + dev->q[1] * dev->q[1] + dev->q[2] * dev->q[2] + dev->q[3] * dev->q[3]);
    dev->q[0] *= recip_norm;
    dev->q[1] *= recip_norm;
    dev->q[2] *= recip_norm;
    dev->q[3] *= recip_norm;

    return 0;
}

/* ------------------------------------------------------------------ */
void mpu6050_get_quaternion(const mpu6050_t *dev, float q[4])
{
    if (dev == NULL || q == NULL) return;
    q[0] = dev->q[0];
    q[1] = dev->q[1];
    q[2] = dev->q[2];
    q[3] = dev->q[3];
}

void mpu6050_get_euler(const mpu6050_t *dev, float *roll, float *pitch, float *yaw)
{
    float w, x, y, z;
    const float rad2deg = 57.295779513f;

    if (dev == NULL) return;
    w = dev->q[0]; x = dev->q[1]; y = dev->q[2]; z = dev->q[3];

    if (roll)
        *roll  = atan2f(2.0f * (w * x + y * z),
                        1.0f - 2.0f * (x * x + y * y)) * rad2deg;
    if (pitch) {
        float s = 2.0f * (w * y - z * x);
        if (s >  1.0f) s =  1.0f;
        if (s < -1.0f) s = -1.0f;
        *pitch = asinf(s) * rad2deg;
    }
    if (yaw)
        *yaw   = atan2f(2.0f * (w * z + x * y),
                        1.0f - 2.0f * (y * y + z * z)) * rad2deg;
}
