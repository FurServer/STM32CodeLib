#include "kalman.h"
#include <math.h>

// 协方差下限
// 防止float下溢到0导致滤波器"冻死"
#define KALMAN_P_MIN  1e-6f

int32_t kalman_filter(kalman_t* filter, int32_t input)
{
    const float z = (float)input;

    /* 预测 */
    /* x不变, 仅增加过程噪声到协方差 */
    filter->p += filter->q;

    /* 更新 */
    // 卡尔曼增益: k = p / (p + r)
    const float k = filter->p / (filter->p + filter->r);

    // 状态更新: x = x + k · (z − x)
    filter->x += k * (z - filter->x);

    // 协方差更新: p = (1 − k) · p
    filter->p *= (1.0f - k);

    // 防止p下溢到0, 钳位到一个极小正值, 保证滤波器始终对测量值有最低信任度
    if (filter->p < KALMAN_P_MIN) { filter->p = KALMAN_P_MIN; }

    return (int32_t)filter->x;
}

void kalman_reset(kalman_t* filter, float init_val)
{
    filter->x = init_val;
    filter->p = 1.0f;
}
