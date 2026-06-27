#ifndef LOWPASS_H
#define LOWPASS_H

#include "main.h"

/**
 * @brief 一阶低通滤波器 (IIR) 结构体
 *
 * 公式: y[n] = y[n-1] + α · (x[n] − y[n-1])
 * α 越小平滑效果越强, 但响应越慢。
 */
typedef struct
{
    float alpha;         /**< 滤波系数, 0~1, 越小越平滑 */
    float last_output;   /**< 上一次滤波输出值 */
} lowpass_t;

/**
 * @brief 声明并初始化低通滤波器
 *
 * @param   name      滤波器变量名
 * @param   _alpha    滤波系数, 0~1, 越小越平滑
 *
 * @code
 * LOWPASS_DEFINE(my_lp, 0.10f);
 * int32_t result = lowpass_filter(&my_lp, new_value);
 * @endcode
 */
#define LOWPASS_DEFINE(name, _alpha)                \
    lowpass_t name = {                               \
        .alpha = (_alpha),                           \
        .last_output = 0.0f                          \
    }

/**
 * @brief 一阶低通滤波
 *
 * 对输入数据进行一阶 IIR 低通滤波。
 *
 * @param   filter  指向滤波器状态结构体指针
 * @param   input   原始输入值
 *
 * @retval  int32_t 滤波后的输出值
 */
int32_t lowpass_filter(lowpass_t* filter, int32_t input);

/**
 * @brief 重置低通滤波器状态
 *
 * @param   filter  指向滤波器状态结构体指针
 */
void lowpass_reset(lowpass_t* filter);

#endif /* LOWPASS_H */
