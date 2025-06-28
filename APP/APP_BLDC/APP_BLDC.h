#ifndef __APP_BLDC_H__
#define __APP_BLDC_H__

#include "Int_BLDC.h"
#include "Int_EEPROM.h"

extern uint8_t bldc_id;

void APP_BLDC_Control(int16_t set_nums);

void APP_BLDC_Start(void);
void APP_BLDC_Stop(void);

void APP_BLDC_ID_Add(void);

void APP_BLDC_ID_Sub(void);

void APP_BLDC_ID_Init(void);

#endif /* __APP_BLDC_H__ */