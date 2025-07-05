#ifndef __INT_CAN_H__
#define __INT_CAN_H__

#include "main.h"
#include "can.h"

// 声明结构体存放CAN的消息
typedef struct
{
    uint32_t id;
    uint8_t data[8];
    uint8_t len;
} CAN_MSG;

void Int_CAN_Init(void);

void Int_CAN_Send(uint32_t id, uint8_t *data, uint16_t len);

void Int_CAN_Receive(CAN_MSG *msgs, uint8_t *msg_count);

#endif /* __INT_CAN_H__ */
