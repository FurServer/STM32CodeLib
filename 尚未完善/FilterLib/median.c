#include "median.h"

int32_t median_filter(median_t* filter, int32_t input)
{
    const uint8_t n = filter->window;

    /* 环形写入采样缓冲区 */
    filter->buf[filter->index] = input;
    filter->index = (filter->index + 1) % n;
    if (filter->filled < n) { filter->filled++; }

    /* 缓冲区未填满时, 直接返回输入值 */
    if (filter->filled < n) return input;

    // 复制缓冲区到临时数组并排序, 取中位数
    int32_t sorted[MEDIAN_MAX_WINDOW];
    for (uint8_t i = 0; i < n; i++) sorted[i] = filter->buf[i];

    // 简单插入排序 (窗口很小, 5~9)
    for (uint8_t i = 1; i < n; i++)
    {
        const int32_t key = sorted[i];
        int8_t j = (int8_t)i - 1;
        while (j >= 0 && sorted[j] > key)
        {
            sorted[j + 1] = sorted[j];
            j--;
        }
        sorted[j + 1] = key;
    }

    // 返回中位数
    return sorted[n / 2];
}

void median_reset(median_t* filter)
{
    filter->index = 0;
    filter->filled = 0;
}
