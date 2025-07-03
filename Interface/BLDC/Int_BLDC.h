#ifndef __INT_BLDC_H__
#define __INT_BLDC_H__

#include "tim.h"

extern uint8_t bldc_dir;
extern uint8_t bldc_status;
// 转一圈的时间
extern uint32_t hall_count_final;

void Int_BLDC_Start(void);

void Int_BLDC_Stop(void);

uint8_t Int_BLDC_GetHall(void);

void Int_BLDC_Control(uint8_t dir, uint16_t target_speed);

#endif /* __INT_BLDC_H__ */
