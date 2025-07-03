#include "APP_BLDC.h"
#include "stdlib.h"

uint8_t bldc_id = 1;
PID_Struct pid;
int16_t target_speed = 0;
int8_t target_dir    = 0;
uint16_t bldc_speed  = 0;
void APP_BLDC_Init(void)
{
    // 初始化ID
    APP_BLDC_ID_Init();
    // 初始化PID参数
    Com_PID_Init(&pid, 0.015, 0.03, 0.0);
}

void APP_BLDC_Speed_Update()
{
    if (target_speed >= 0) {
        target_dir = 1;
    } else {
        target_dir = 0;
    }
    Int_BLDC_Control(target_dir, (uint16_t)pid.output);
}

void APP_BLDC_Start(void)
{
    Com_PID_Rest(&pid);
    Int_BLDC_Start();
}
void APP_BLDC_Stop(void)
{
    bldc_speed = 0;
    Int_BLDC_Stop();
    Com_PID_Rest(&pid);
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

static uint32_t last_PID_time = 0;
void HAL_IncTick(void)
{
    uwTick += uwTickFreq;
    if (bldc_status == 1) {
        if (uwTick - last_PID_time >= 50) {
            last_PID_time = uwTick;
            bldc_speed    = (540000.0 / hall_count_final);
            // printf("%d,%d,%d\n", target_speed, (uint16_t)pid.output, bldc_speed);
            Com_PID_Update(&pid, abs(target_speed) - bldc_speed);
            // // 限制PID计算结果的上下限
            if (pid.output > 1000) {
                pid.output = 1000;
            } else if (pid.output < 50) {
                pid.output = 50;
            }
        }
        APP_BLDC_Speed_Update();
    }
}
