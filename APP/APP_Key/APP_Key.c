#include "APP_Key.h"
#include "Int_EEPROM.h"
#include "Int_oled.h"

uint8_t key_value = 0;
extern uint8_t page_flag;
extern uint8_t machine_id;
extern int16_t set_nums;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    // 1. 判断中断是否为外部中端触发
    if (GPIO_Pin == KEY_1_Pin || GPIO_Pin == KEY_2_Pin || GPIO_Pin == KEY_3_Pin || GPIO_Pin == KEY_4_Pin) {
        // 2. 消抖
        HAL_Delay(10);
        // 3. 判断哪个按键按下
        if (HAL_GPIO_ReadPin(KEY_1_GPIO_Port, KEY_1_Pin) == GPIO_PIN_RESET) {
            key_value = 1;
            // 处理 KEY_1 按键按下事件
        } else if (HAL_GPIO_ReadPin(KEY_2_GPIO_Port, KEY_2_Pin) == GPIO_PIN_RESET) {
            key_value = 2;
            // 处理 KEY_2 按键按下事件
        } else if (HAL_GPIO_ReadPin(KEY_3_GPIO_Port, KEY_3_Pin) == GPIO_PIN_RESET) {
            key_value = 3;
            // 处理 KEY_3 按键按下事件
        } else if (HAL_GPIO_ReadPin(KEY_4_GPIO_Port, KEY_4_Pin) == GPIO_PIN_RESET) {
            key_value = 4;
            // 处理 KEY_4 按键按下事件
        }
    }
}

void APP_Key_Process(void)
{
    switch (key_value) {
        case 1:
            debug_printfln("KEY_1");
            break;
        case 2:
            debug_printfln("KEY_2");
            if (page_flag == 1) {
                // id+1
                machine_id++;
                if (machine_id > 99) {
                    machine_id = 99;
                }
                // 存储ID
                Int_EEPROM_Write(0x01, machine_id);
            } else {
                // 转速增加
                set_nums += 50;
                if (set_nums > 600) {
                    set_nums = 600;
                }
            }
            break;
        case 3:
            debug_printfln("KEY_3");
            if (page_flag == 1) {
                machine_id--;
                if (machine_id < 1) {
                    machine_id = 1;
                }
                Int_EEPROM_Write(0x01, machine_id);
            } else {
                // 转速降低
                set_nums -= 50;
                if (set_nums < -600) {
                    set_nums = -600;
                }
            }
            break;
        case 4: // 切换页面
            debug_printfln("KEY_4");
            // 按键4被按下 => 处理切换页面
            page_flag = (page_flag + 1) % 2;
            key_value = 0;
            Int_OLED_Clear();
            break;
        default:
            break;
    }
    key_value = 0;
}