#include "Int_CAN.h"

#define CAN_FILTER_EXTID_H(EXTID) ((uint16_t)(((EXTID) >> 13) & 0xFFFF))
#define CAN_FILTER_EXTID_L(EXTID) ((uint16_t)(((uint32_t)(EXTID) << 3U) | ((uint8_t)CAN_ID_EXT)))

void Int_CAN_Init(void)
{

    // 1. 配置过滤器
    CAN_FilterTypeDef sFilterConfig;

    // 过滤器编号
    sFilterConfig.FilterBank           = 0;                     // 0-13
    sFilterConfig.FilterFIFOAssignment = CAN_FILTER_FIFO0;      // 绑定到队列0
    sFilterConfig.FilterMode           = CAN_FILTERMODE_IDMASK; // 标识符屏蔽位模式
    sFilterConfig.FilterScale          = CAN_FILTERSCALE_32BIT; // 32位宽

    uint32_t ext_id                    = 0x1317C02;             // 20020226 十进制
    sFilterConfig.FilterIdHigh = CAN_FILTER_EXTID_H(ext_id); // 高13位左移3位
    sFilterConfig.FilterIdLow  = CAN_FILTER_EXTID_L(ext_id); // 低18位

    sFilterConfig.FilterMaskIdHigh = 0xFFFF; // 32位MASK高位
    sFilterConfig.FilterMaskIdLow  = 0xFFFF; // 32位MASK低位


    sFilterConfig.FilterActivation = ENABLE; // 使能过滤器

    HAL_CAN_ConfigFilter(&hcan, &sFilterConfig);

    // 2. 启动can
    HAL_CAN_Start(&hcan);
}

void Int_CAN_Send(uint32_t id, uint8_t *data, uint16_t len)
{
    // 1. CAN外设一共有3个发送邮箱
    while (HAL_CAN_GetTxMailboxesFreeLevel(&hcan) == 0) {
        HAL_Delay(100);
    }

    // 2. 调用发送数据
    CAN_TxHeaderTypeDef TxHeader;
    TxHeader.StdId = id;           // 消息ID
    TxHeader.IDE   = CAN_ID_STD;   // 标准格式
    TxHeader.RTR   = CAN_RTR_DATA; // 数据帧
    TxHeader.DLC   = len;          // 数据长度
    uint32_t TxMailbox;
    HAL_CAN_AddTxMessage(&hcan, &TxHeader, data, &TxMailbox); // 数据
}

void Int_CAN_Receive(CAN_MSG *msgs, uint8_t *msg_count)
{
    // 1. 查询收件箱中是否有消息
    *msg_count = HAL_CAN_GetRxFifoFillLevel(&hcan, CAN_RX_FIFO0);

    // 2. 收到消息存放到msgs
    for (uint8_t i = 0; i < *msg_count; i++) {
        CAN_RxHeaderTypeDef Rxheader;
        HAL_CAN_GetRxMessage(&hcan, CAN_RX_FIFO0, &Rxheader, msgs[i].data);
        // 统一使用标准格式的数据帧
        // msgs[i].id  = Rxheader.StdId;
        msgs[i].id  = Rxheader.ExtId;
        msgs[i].len = Rxheader.DLC;
    }
}
