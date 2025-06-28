#include "APP_Display.h"
#include "Int_EEPROM.h"
/**
 * 展示页面0:
 *  第一行:    尚硅谷电机项目
 *  第二行:    设置的速度
 *  第三行:    编码器读取计算的速度
 * 展示页面1:
 *     modbus_RS485 从设备的ID
 */
uint8_t page_flag  = 0;
uint8_t machine_id = 1;
int16_t set_nums   = 0;

uint16_t encoder_speed = 0;

/**
 * @brief 电机ID的初始化
 *
 */
void APP_ID_Init(void)
{
    // ID值存储在0x01位置 => 把0x00 => 188
    // 初始化之后,先读取0x00位置的值 判断当前是否是之前存储的id值
    uint8_t tmp = Int_EEPROM_Read(0x00);
    if (tmp == 188) {
        machine_id = Int_EEPROM_Read(0x01);
    } else {
        // 之前没有存储过ID值,则存储
        Int_EEPROM_Write(0x00, 188);
        Int_EEPROM_Write(0x01, machine_id);
    }
}

/**
 * @brief 初始化OLED屏幕
 *
 */
void APP_Display_Init(void)
{
    // 初始化OLED屏幕
    Int_OLED_Init();
}

/**
 * @brief 轮询调用屏幕展示方法
 *
 */
void APP_Display_Show(void)
{
    // if (page_flag == 1) {
    //     // 展示设备ID页面
    //     // 1. 展示 ID标识
    //     Int_OLED_ShowString(30, 16, "ID:", 16, 1);
    //     // 2. 展示ID值
    //     Int_OLED_ShowNum(58, 16, machine_id, 2, 16, 1);
    // } else {
    //     // 展示速度控制页面
    //     // 1. 展示标题
    //     for (uint8_t i = 0; i < 7; i++) {
    //         Int_OLED_ShowChinese(8 + 16 * i, 0, i, 16, 1);
    //     }
    //     // 2. set_nums
    //     Int_OLED_ShowString(0, 16, "set_nums:", 16, 1);
    //     // 3. 判断设置速度的正负号
    //     if (set_nums >= 0) {
    //         Int_OLED_ShowChar(75, 16, '+', 16, 1);
    //         Int_OLED_ShowNum(83, 16, set_nums, 4, 16, 1);
    //     } else {
    //         Int_OLED_ShowChar(75, 16, '-', 16, 1);
    //         Int_OLED_ShowNum(83, 16, -set_nums, 4, 16, 1);
    //     }
    //     // 4. 显示编码器测量的速度值
    //     Int_OLED_ShowString(0, 32, "speed:", 16, 1);
    //     Int_OLED_ShowNum(83, 32, encoder_speed, 4, 16, 1);
    // }

    switch (page_flag) {
        case 0:
            // 展示速度控制页面
            // 1. 展示标题
            for (uint8_t i = 0; i < 7; i++) {
                Int_OLED_ShowChinese(8 + 16 * i, 0, i, 16, 1);
            }

            // 2. set_nums
            Int_OLED_ShowString(0, 16, "set_nums:", 16, 1);

            // 3. 判断设置速度的正负号
            if (set_nums >= 0) {
                Int_OLED_ShowChar(75, 16, '+', 16, 1);
                Int_OLED_ShowNum(83, 16, set_nums, 4, 16, 1);
            } else {
                Int_OLED_ShowChar(75, 16, '-', 16, 1);
                Int_OLED_ShowNum(83, 16, -set_nums, 4, 16, 1);
            }

            // 4. 显示编码器测量的速度值
            Int_OLED_ShowString(0, 32, "speed:", 16, 1);
            Int_OLED_ShowNum(83, 32, encoder_speed, 4, 16, 1);
            break;
        case 1:
            // 展示设备ID页面
            // 1. 展示 ID标识
            Int_OLED_ShowString(30, 16, "ID:", 16, 1);
            // 2. 展示ID值
            Int_OLED_ShowNum(58, 16, machine_id, 2, 16, 1);
            break;
        default:
            break;
    }

    // 5. 刷新屏幕
    Int_OLED_Refresh();
}
