/**
 * @file    example.c
 * @brief   使用示例
 * @note    可能需要在cmake中
 *          加上set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -Wl,-u,_printf_float")
 *          以正确编译浮点数snprintf
 */

#include "mpu6050.h"

//设备句柄
static mpu6050_t imu;
// 四元数 w,x,y,z
static float g_quat[4] = {1.0f, 0.0f, 0.0f, 0.0f};
// 姿态角
static float g_pitch = 0.0f;
static float g_roll = 0.0f;
static float g_yaw = 0.0f;
// 加速度
static float g_acc[3] = {0};
// 角速度
static float g_dps[3] = {0};
// 上次时间戳
static uint32_t g_last_tick = 0;

// u8g2 (可选)
u8g2_t u8g2;
char oled[32];

int main(void)
{
    // 初始化
    mpu6050_init(&imu, MPU6050_ACCEL_2G, MPU6050_GYRO_2000DPS);
    
    // 零偏校准
    mpu6050_calibrate_gyro(&imu, 1000);
    printf("bias = %+.3f, %+.3f, %+.3f dps\n", imu.gyro_bias[0], imu.gyro_bias[1], imu.gyro_bias[2]);
    
    g_last_tick = HAL_GetTick();
    
    while(1)
    {
        float dt;
        uint32_t now = HAL_GetTick();

		// 增量时间安全钳位计算
        dt = (float)(now - g_last_tick) / 1000.0f;
        g_last_tick = now;
        if (dt > 0.5f || dt <= 0.0f) dt = 0.01f;

        if (mpu6050_update(&imu, dt) == 0)
        {
            // 读取原始加速度/角速度
            mpu6050_read(&imu, g_acc, g_dps);
            // 获取四元数 w,x,y,z
            mpu6050_get_quaternion(&imu, g_quat);
            // 由四元数解算欧拉角 (单位: 度)
            mpu6050_get_euler(&imu, &g_roll, &g_pitch, &g_yaw);
        }

		// 显示 (可选)
		u8g2_FirstPage(&u8g2);
        do
        {
            u8g2_SetFontMode(&u8g2, 1);
            u8g2_SetFontDirection(&u8g2, 0);
            u8g2_SetFont(&u8g2, u8g2_font_t0_16_mr);
            snprintf(oled, sizeof(oled), "P%7.02f", g_pitch);
            u8g2_DrawStr(&u8g2, 0, 10, oled);
            snprintf(oled, sizeof(oled), "R%7.02f", g_roll);
            u8g2_DrawStr(&u8g2, 0, 23, oled);
            snprintf(oled, sizeof(oled), "Y%7.02f", g_yaw);
            u8g2_DrawStr(&u8g2, 0, 36, oled);
            u8g2_SetFont(&u8g2, u8g2_font_5x7_mr);
            snprintf(oled, sizeof(oled), "%+.2f%+.2f%+.2f%+.2f", g_quat[0], g_quat[1], g_quat[2], g_quat[3]);
            u8g2_DrawStr(&u8g2, 0, 49, oled);
        }
        while (u8g2_NextPage(&u8g2));

    }
}