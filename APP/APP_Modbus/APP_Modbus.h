#ifndef __APP_MODBUS_H__
#define __APP_MODBUS_H__

#include "mb.h"
#include "APP_BLDC.h"

void APP_Modbus_Start(uint8_t id);

void APP_Modbus_Process(void);

void APP_Modbus_Msg_Update(uint8_t x);

#endif /* __APP_MODBUS_H__ */