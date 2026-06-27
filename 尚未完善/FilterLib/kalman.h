#ifndef KALMAN_H
#define KALMAN_H

#include "main.h"

/**
 * @brief 一维卡尔曼滤波器结构体
 *
 *  公式:
 *  预测:  x = x,  p = p + q
 *  更新:  k = p / (p + r),  x = x + k·(z − x),  p = (1 − k)·p
 *
 * - Q (过程噪声协方差): 描述系统模型的可信度, 越大越信任测量值
 * - R (测量噪声协方差): 描述传感器的噪声水平, 越大越信任预测值
 */
typedef struct
{
    float q;    // 过程噪声协方差 (推荐 0.001 ~ 0.1)
    float r;    // 测量噪声协方差 (推荐 0.1  ~ 10)
    float x;    // 状态估计值
    float p;    // 估计误差协方差
} kalman_t;

/**
 * @brief 编译时声明并初始化卡尔曼滤波器
 *
 * @param   name       滤波器变量名
 * @param   _q         过程噪声协方差
 * @param   _r         测量噪声协方差
 * @param   init_val   初始估计值
 *
 * @note    Q 越大越信任测量值 (响应快但平滑弱);
 *          R 越大越不信任测量值 (平滑强但响应慢)
 *
 * @code
 * KALMAN_DEFINE(my_kf, 0.01f, 2.0f, 0);
 * int32_t result = kalman_filter(&my_kf, new_value);
 * @endcode
 */
#define KALMAN_DEFINE(name, _q, _r, init_val)            \
    kalman_t name = {                                    \
        .q = (_q),                                       \
        .r = (_r),                                       \
        .x = (float)(init_val),                          \
        .p = 1.0f                                        \
    }

/**
 * @brief 一维卡尔曼滤波
 *
 * @param   filter  指向滤波器状态结构体指针
 * @param   input   原始测量值
 *
 * @retval  int32_t 滤波后的输出值
 */
int32_t kalman_filter(kalman_t* filter, int32_t input);

/**
 * @brief 重置卡尔曼滤波器状态
 *
 * 将估计值重置为指定值, 协方差恢复为初始值。
 *
 * @param   filter    指向滤波器状态结构体指针
 * @param   init_val  新的初始估计值
 */
void kalman_reset(kalman_t* filter, float init_val);

#endif /* KALMAN_H */
