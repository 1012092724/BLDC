#include "APP_BLDC.h"

uint8_t bldc_id = 1;

void APP_BLDC_Control(int16_t set_nums)
{
    Int_BLDC_Control(set_nums);
}

void APP_BLDC_Start(void)
{
    Int_BLDC_Start();
}
void APP_BLDC_Stop(void)
{
    Int_BLDC_Stop();
}

void APP_BLDC_ID_Add(void)
{
    bldc_id++;
    if (bldc_id > 100) {
        bldc_id = 99;
    }
    Int_EEPROM_Write(0x01, bldc_id);
}

void APP_BLDC_ID_Sub(void)
{
    bldc_id--;
    if (bldc_id < 1) {
        bldc_id = 1;
    }
    Int_EEPROM_Write(0x01, bldc_id);
}

void APP_BLDC_ID_Init(void)
{
    // ID值存储在0x01位置 => 把0x00 => 188
    // 初始化之后,先读取0x00位置的值 判断当前是否是之前存储的id值
    uint8_t tmp = Int_EEPROM_Read(0x00);
    if (tmp == 188) {
        bldc_id = Int_EEPROM_Read(0x01);
    } else {
        // 之前没有存储过ID值,则存储
        Int_EEPROM_Write(0x00, 188);
        Int_EEPROM_Write(0x01, bldc_id);
    }
}