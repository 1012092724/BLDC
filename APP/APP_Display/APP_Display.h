#ifndef __APP_DISPLAY_H__
#define __APP_DISPLAY_H__

#include "Com_Debug.h"
#include "Int_oled.h"

void APP_Display_Init(void);

void APP_Display_Show(void);

extern int16_t set_nums;
extern uint8_t page_flag;
#endif /* __APP_DISPLAY_H__ */