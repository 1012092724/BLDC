#include "Com_PID.h"

/**
 * 初始化PID控制器
 *
 * @param pid PID结构体的指针，用于存储PID控制器的相关参数和状态
 * @param kp 比例增益，决定比例项的反应速度
 * @param ki 积分增益，决定积分项累加速度和消除稳态误差的能力
 * @param kd 微分增益，决定微分项对误差变化率的反应速度
 * @param integral_limit 积分限制，防止积分饱和
 * @param output_limit 输出限制，确保PID控制器的输出在有效范围内
 */
void Com_PID_Init(PID_Struct *pid, float kp, float ki, float kd)
{
    // 初始化比例增益
    pid->kp = kp;
    // 初始化积分增益
    pid->ki = ki;
    // 初始化微分增益
    pid->kd = kd;
    // 初始化误差为0
    pid->error = 0;
    // 初始化上一次误差为0
    pid->error_last = 0;
    // 初始化误差累加和为0
    pid->error_sum = 0;
    // 初始化输出为0
    pid->output = 60;
}

void Com_PID_Rest(PID_Struct *pid)
{
    // 初始化误差为0
    pid->error = 0;
    // 初始化上一次误差为0
    pid->error_last = 0;
    // 初始化误差累加和为0
    pid->error_sum = 0;
    // 初始化输出为0
    pid->output = 50;
}
void Com_PID_Update(PID_Struct *pid, int16_t error)
{

    pid->error = error;

    // 首次修改上一次的误差值
    if (pid->error_last == 0) {
        pid->error_last = pid->error;
    }
    // 计算误差积分
    pid->error_sum += pid->error;

    // PID公式
    pid->output = pid->kp * pid->error + pid->ki * pid->error_sum + pid->kd * (pid->error - pid->error_last);

    // 更新误差值
    pid->error_last = pid->error;
}
