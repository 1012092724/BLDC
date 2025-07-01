#ifndef __COM_PID_H__
#define __COM_PID_H__
#include "main.h"
#include "APP_BLDC.h"
#include "stdlib.h"

typedef struct {
    float kp, ki, kd;
    float error;
    float error_last;
    float error_sum;
    float output;
} PID_Struct;

void Com_PID_Init(PID_Struct *pid, float kp, float ki, float kd);

void Com_PID_Rest(PID_Struct *pid);

void Com_PID_Update(PID_Struct *pid, int16_t error);

#endif /* __COM_PID_H__ */
