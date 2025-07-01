#ifndef __INT_BLDC_H__
#define __INT_BLDC_H__

#include "tim.h"

// #define BLDC_SPEED 60000.0 / (hall_count_final / 9.0)
#define BLDC_SPEED hall_count_final > 0 ? (540000 / hall_count_final) : 0

extern uint8_t bldc_status;
// 转一圈的时间
extern uint32_t hall_count_final;

void Int_BLDC_Start(void);

void Int_BLDC_Stop(void);

uint8_t Int_BLDC_GetHall(void);

void Int_BLDC_Control(uint8_t dir, uint16_t target_speed);

#endif /* __INT_BLDC_H__ */
