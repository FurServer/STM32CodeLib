/**
 ******************************************************************************
 * @file    pid.h
 * @brief   通用 PID 控制器库 (位置式 + 增量式, 带抗积分饱和)
 *
 * 使用流程: 
 *   PID_t pid;
 *   PID_Init(&pid, PID_MODE_POSITION, 1.0f, 0.1f, 0.05f, dt);   // 配置参数
 *   PID_SetOutputLimit(&pid, -1000.0f, 1000.0f);                // 配置输出限幅
 *   PID_SetIntegralLimit(&pid, -500.0f, 500.0f);                // 配置积分限幅
 *   ...
 *   float out = PID_Compute(&pid, target, measure);            // 周期性调用
 ******************************************************************************
 */

#ifndef LIB_PID_H
#define LIB_PID_H

#ifdef __cplusplus
extern "C" {
#endif

// PID 算法形态
typedef enum
{
    PID_MODE_POSITION = 0,  // 位置式 PID: 输出控制量绝对值
    PID_MODE_INCREMENT      // 增量式 PID: 输出本次增量
} PID_Mode_t;

// 抗积分饱和 (Anti-Windup)策略
typedef enum
{
    PID_AW_NONE = 0,        // 不做处理
    PID_AW_INTEGRAL_LIMIT,  // 积分限幅: 约束积分累加值 [i_min, i_max]
    PID_AW_CLAMP,           // 条件积分: 饱和但仍存在积分时停止积分
    PID_AW_BACK_CALC        // 反向计算: 用饱和误差按 Kb 反馈回积分
} PID_AntiWindup_t;

// PID 控制器对象
typedef struct
{
    /* ---- 基本参数 ---- */
    float Kp;               // 比例增益
    float Ki;               // 积分增益, 已包含采样周期 dt
    float Kd;               // 微分增益
    float dt;               // 采样周期, 单位毫秒(ms)

    PID_Mode_t       mode;  // 位置式/增量式
    PID_AntiWindup_t aw;    // 抗积分饱和策略

    /* ---- 限幅参数 ---- */
    float out_min;          // 输出下限
    float out_max;          // 输出上限
    float i_min;            // 积分项下限 (仅位置式+PID_AW_INTEGRAL_LIMIT使用)
    float i_max;            // 积分项上限
    float Kb;               // 反向计算抗饱和系数 (仅PID_AW_BACK_CALC使用)

    /* ---- 运行时内部状态 ---- */
    float integral;         // 位置式: 积分累加值
    float error_prev;       // 上一次偏差 e[k-1]
    float error_pprev;      // 上上次偏差 e[k-2]
    float output;           // 上一次输出

    bool  first_run;        //首次运行标志, 用于避免首拍微分冲击
} PID_t;

/* ====================== 接口函数 ====================== */

/**
 * @brief 初始化 PID 控制器
 * @param pid   控制器对象指针
 * @param mode  位置式或增量式
 * @param Kp/Ki/Kd  三个增益
 * @param dt    采样周期(ms)
 */
void PID_Init(PID_t *pid, PID_Mode_t mode, float Kp, float Ki, float Kd, float dt);

/**
 * @brief 在线修改三个增益
 */
void PID_SetGains(PID_t *pid, float Kp, float Ki, float Kd);

/**
 * @brief 设置输出限幅 [min, max]
 */
void PID_SetOutputLimit(PID_t *pid, float out_min, float out_max);

/**
 * @brief 设置积分限幅 [min, max] 并选用 PID_AW_INTEGRAL_LIMIT 策略
 */
void PID_SetIntegralLimit(PID_t *pid, float i_min, float i_max);

/**
 * @brief 选择抗积分饱和策略
 * @param Kb 仅 PID_AW_BACK_CALC 需要, 其它策略可传 0
 */
void PID_SetAntiWindup(PID_t *pid, PID_AntiWindup_t aw, float Kb);

/**
 * @brief 清零内部状态
 */
void PID_Reset(PID_t *pid);

/**
 * @brief 执行一次 PID 运算 (使用 PID_Init 时设定的固定 dt)
 * @param target   目标值
 * @param measure  实际值
 * @return 限幅后的控制输出
 *
 * @note 仅当你能保证“严格固定周期”调用 (如定时器中断) 时才用本函数。
 */
float PID_Compute(PID_t *pid, float target, float measure);

/**
 * @brief 执行一次 PID 运算, 并使用“本次实测的时间间隔 dt”
 * @param target   目标值
 * @param measure  实际值
 * @param dt       本次距上次调用经过的真实时间(ms), 必须 > 0
 * @return 限幅后的控制输出
 *
 */
float PID_ComputeDt(PID_t *pid, float target, float measure, float dt);

#ifdef __cplusplus
}
#endif

#endif
