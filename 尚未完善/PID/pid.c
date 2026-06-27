/**
 ******************************************************************************
 * @file    pid.c
 * @brief   通用 PID 控制器库
 ******************************************************************************
 */

#include "pid.h"

// 内部工具, 约束数值
static inline float clampf(float value, float min, float max)
{
    if (value > max) return max;
    if (value < min) return min;
    return value;
}

/* ---------------- 配置类接口 ---------------- */

void PID_Init(PID_t *pid, PID_Mode_t mode, float Kp, float Ki, float Kd, float dt)
{
    if (pid == 0) return;

    pid->Kp   = Kp;
    pid->Ki   = Ki;
    pid->Kd   = Kd;
    pid->dt   = (dt > 0.0f) ? dt : 1.0f;    //防止除零
    pid->mode = mode;

    // 默认不限幅, 用一个很大的范围近似无穷, 不抗饱和
    pid->aw      = PID_AW_NONE;
    pid->out_min = -3.4e38f;
    pid->out_max =  3.4e38f;
    pid->i_min   = -3.4e38f;
    pid->i_max   =  3.4e38f;
    pid->Kb      =  0.0f;

    PID_Reset(pid);
}

void PID_SetGains(PID_t *pid, float Kp, float Ki, float Kd)
{
    if (pid == 0) return;
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;
}

void PID_SetOutputLimit(PID_t *pid, float out_min, float out_max)
{
    if (pid == 0) return;
    pid->out_min = out_min;
    pid->out_max = out_max;
}

void PID_SetIntegralLimit(PID_t *pid, float i_min, float i_max)
{
    if (pid == 0) return;
    pid->i_min = i_min;
    pid->i_max = i_max;
    // 设置积分限幅后, 默认启用对应的抗饱和策略
    if (pid->aw == PID_AW_NONE)
    {
        pid->aw = PID_AW_INTEGRAL_LIMIT;
    }
}

void PID_SetAntiWindup(PID_t *pid, PID_AntiWindup_t aw, float Kb)
{
    if (pid == 0) return;
    pid->aw = aw;
    pid->Kb = Kb;
}

void PID_Reset(PID_t *pid)
{
    if (pid == 0) return;
    pid->integral    = 0.0f;
    pid->error_prev  = 0.0f;
    pid->error_pprev = 0.0f;
    pid->output      = 0.0f;
    pid->first_run   = true;
}

/* ---------------- 位置式 PID ----------------
 *
 * 数学形式 (连续域): 
 *     u(t) = Kp*e(t) + Ki*∫e(τ)dτ + Kd*de(t)/dt
 *
 * 离散化 (采样周期 dt): 
 *     P = Kp * e[k]
 *     I = Ki * Σ(e[i]*dt)
 *     D = Kd * (e[k]-e[k-1]) / dt
 *     u[k] = P + I + D
 *
 */
static float PID_ComputePosition(PID_t *pid, float error)
{
    float P, I, D;
    float out_unsat, out_sat;

    // 比例项
    P = pid->Kp * error;

    // 微分项
    if (pid->first_run) D = 0.0f;
    else D = pid->Kd * (error - pid->error_prev) / pid->dt;

    // 积分项
    float integral_try = pid->integral + pid->Ki * error * pid->dt;

    // 策略: 积分限幅
    if (pid->aw == PID_AW_INTEGRAL_LIMIT)
    {
        integral_try = clampf(integral_try, pid->i_min, pid->i_max);
    }

    I = integral_try;

    // 求未饱和输出
    out_unsat = P + I + D;

    // 输出饱和限幅
    out_sat = clampf(out_unsat, pid->out_min, pid->out_max);

    // 策略: 条件积分
    if (pid->aw == PID_AW_CLAMP)
    {
        bool saturated   = (out_unsat != out_sat);
        bool same_sign   = ((error > 0.0f) == (out_unsat > 0.0f));
        if (saturated && same_sign)
        {   // 撤销本次积分
            I            = pid->integral;
            out_unsat    = P + I + D;
            out_sat      = clampf(out_unsat, pid->out_min, pid->out_max);
            integral_try = pid->integral;
        }
    }

    // 策略: 反向计算
    if (pid->aw == PID_AW_BACK_CALC)
    {
        integral_try += pid->Kb * (out_sat - out_unsat) * pid->dt;
    }

    /* --- 提交内部状态 --- */
    pid->integral   = integral_try;
    pid->error_prev = error;
    pid->output     = out_sat;
    pid->first_run  = false;

    return out_sat;
}

/* ---------------- 增量式 PID ----------------
 *
 * 对位置式做一次差分: u[k]-u[k-1] = Δu[k]
 *
 *     Δu[k] = Kp*(e[k]-e[k-1])
 *           + Ki*e[k]*dt
 *           + Kd*(e[k]-2*e[k-1]+e[k-2]) / dt
 *
 *     u[k] = u[k-1] + Δu[k]
 *
 */
static float PID_ComputeIncrement(PID_t *pid, float error)
{
    float dP, dI, dD, delta;
    float out_unsat, out_sat;

    if (pid->first_run)
    {
        // 首拍: 历史误差未知, 令其等于当前误差, 使增量近似0, 避免冲击
        pid->error_prev  = error;
        pid->error_pprev = error;
    }

    dP = pid->Kp * (error - pid->error_prev);
    dI = pid->Ki * error * pid->dt;
    dD = pid->Kd * (error - 2.0f * pid->error_prev + pid->error_pprev) / pid->dt;

    delta = dP + dI + dD;

    out_unsat = pid->output + delta;
    out_sat   = clampf(out_unsat, pid->out_min, pid->out_max);

    // 条件积分: 若输出已饱和且增量继续把它往饱和方向推, 则丢弃本拍积分增量 dI
    if (pid->aw == PID_AW_CLAMP || pid->aw == PID_AW_INTEGRAL_LIMIT)
    {
        bool saturated = (out_unsat != out_sat);
        bool pushing_more =
            ((out_unsat > pid->out_max) && (delta > 0.0f)) ||
            ((out_unsat < pid->out_min) && (delta < 0.0f));
        if (saturated && pushing_more)
        {
            delta     = dP + dD;             /* 去掉积分增量 dI */
            out_unsat = pid->output + delta;
            out_sat   = clampf(out_unsat, pid->out_min, pid->out_max);
        }
    }

    // 更新历史误差与输出
    pid->error_pprev = pid->error_prev;
    pid->error_prev  = error;
    pid->output      = out_sat;
    pid->first_run   = false;

    return out_sat;
}

/* ---------------- 统一入口 ---------------- */

float PID_Compute(PID_t *pid, float target, float measure)
{
    if (pid == 0) return 0.0f;

    float error = target - measure;

    if (pid->mode == PID_MODE_INCREMENT)
        return PID_ComputeIncrement(pid, error);
    else
        return PID_ComputePosition(pid, error);
}

float PID_ComputeDt(PID_t *pid, float target, float measure, float dt)
{
    if (pid == 0) return 0.0f;

    if (dt > 0.0f)
        pid->dt = dt;

    return PID_Compute(pid, target, measure);
}
