#include "APP_Key.h"

uint8_t key_value = 0;

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
            if (bldc_status == 0) {
                APP_BLDC_Start();
            } else if (bldc_status == 1) {
                APP_BLDC_Stop();
            }
            break;
        case 2:
            debug_printfln("KEY_2");
            if (page_flag == 1) {
                APP_BLDC_ID_Add(); // ID ++
            } else {
                // 转速增加
                target_speed += 50;
                if (target_speed >= 2000) {
                    target_speed = 2000;
                }
            }
            break;
        case 3:
            debug_printfln("KEY_3");
            if (page_flag == 1) {
                APP_BLDC_ID_Sub(); // ID --
            } else {
                // 转速降低
                target_speed -= 50;
                if (target_speed <= -2000) {
                    target_speed = -2000;
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

    APP_Modbus_Msg_Update(key_value);

    key_value = 0;
}