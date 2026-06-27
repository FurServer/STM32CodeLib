#ifndef MEDIAN_H
#define MEDIAN_H

#include "main.h"

/** @brief 中位值滤波最大窗口大小 (用于内部排序临时数组) */
#define MEDIAN_MAX_WINDOW 16

/**
 * @brief 中位值滤波结构体
 *
 * 对滑动窗口内的数据排序后直接返回中位数，
 * 适合去除偶发性脉冲干扰。
 */
typedef struct
{
    uint8_t window;      // 滑动窗口大小, 建议 5~9 的奇数
    int32_t* buf;        // 指向内部环形缓冲区 (外部分配)
    uint8_t index;       // 环形缓冲区写指针
    uint8_t filled;      // 已填充计数, 用于判断窗口是否填满
} median_t;

/**
 * @brief 声明并初始化中位值滤波器
 *
 * @param   name      滤波器变量名
 * @param   n         滑动窗口大小, 建议 5~9 的奇数 (不能超过 MEDIAN_MAX_WINDOW)
 *
 * @note    自动创建静态缓冲区并初始化结构体
 *
 * @code
 * MEDIAN_DEFINE(my_filter, 7);
 * int32_t result = median_filter(&my_filter, new_value);
 * @endcode
 */
#define MEDIAN_DEFINE(name, n)                  \
    int32_t name##_buf[n] = {0};                \
    median_t name = {                           \
        .window = n,                            \
        .buf = name##_buf                       \
    }

/**
 * @brief 中位值滤波
 *
 * 将新数据加入滑动窗口, 排序后返回中位数。
 *
 * @param   filter  指向滤波器状态结构体指针
 * @param   input   新采样值
 *
 * @retval  int32_t 滤波后的输出值 (窗口中间元素)
 *
 * @note    窗口未填满时直接返回输入值
 */
int32_t median_filter(median_t* filter, int32_t input);

/**
 * @brief 重置中位值滤波器状态
 *
 * 清除缓冲区并重置内部指针, 等同于重新开始。
 *
 * @param   filter  指向滤波器状态结构体指针
 */
void median_reset(median_t* filter);

#endif /* MEDIAN_H */
