#include "Int_BLDC.h"

uint8_t bldc_status = 0;
uint8_t bldc_dir    = 0;
uint16_t dutyCycle  = 0;

// 当前累计值
uint32_t hall_count = 0;
// 最终捕获值  => 12次*1
uint32_t hall_count_final = 0;
uint8_t step_count        = 0;
// 计算电机转速需要的变量
uint8_t current_hall_status = 0;
uint8_t last_hall_status    = 0;
void Int_BLDC_Start(void)
{
    // 1. 控制驱动芯片引脚 YC_SD
    HAL_GPIO_WritePin(YC_SD_GPIO_Port, YC_SD_Pin, GPIO_PIN_SET);

    // 2. 启动TIM8定时器中断
    __HAL_TIM_ENABLE_IT(&htim8, TIM_IT_UPDATE);
    HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_3);

    // 3. 初始化占空比
    __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_1, 0);
    __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_2, 0);
    __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_3, 0);

    //  延时启动 保证电机正常运行
    HAL_Delay(10);

    bldc_status = 1;
}

void Int_BLDC_Stop(void)
{
    // 1. 关闭定时器TIM8
    __HAL_TIM_DISABLE_IT(&htim8, TIM_IT_UPDATE);
    HAL_TIM_PWM_Stop(&htim8, TIM_CHANNEL_1);
    HAL_TIM_PWM_Stop(&htim8, TIM_CHANNEL_2);
    HAL_TIM_PWM_Stop(&htim8, TIM_CHANNEL_3);

    // 2. 清空占空比的值
    __HAL_TIM_SetCompare(&htim8, TIM_CHANNEL_1, 0);
    __HAL_TIM_SetCompare(&htim8, TIM_CHANNEL_2, 0);
    __HAL_TIM_SetCompare(&htim8, TIM_CHANNEL_3, 0);

    // 3. 关闭YC_SD使能驱动芯片  => 如果关闭芯片 会造成再次启动电机会慢一些
    // HAL_GPIO_WritePin(YC_SD_GPIO_Port, YC_SD_Pin, GPIO_PIN_RESET);

    // 4. 标记电机停止
    bldc_status = 0;

    // 5. 清0 编码器测速的参数
    hall_count       = 0;
    hall_count_final = 0;
    step_count       = 0;
}

uint8_t Int_BLDC_GetHall(void)
{
    uint8_t hall_status = 0;
    if (HAL_GPIO_ReadPin(HALL_U_GPIO_Port, HALL_U_Pin) == GPIO_PIN_SET) {
        hall_status |= 0x04;
    } else {
        hall_status &= 0xFB;
    }
    if (HAL_GPIO_ReadPin(HALL_V_GPIO_Port, HALL_V_Pin) == GPIO_PIN_SET) {
        hall_status |= 0x02;
    } else {
        hall_status &= 0xFD;
    }
    if (HAL_GPIO_ReadPin(HALL_W_GPIO_Port, HALL_W_Pin) == GPIO_PIN_SET) {
        hall_status |= 0x01;
    } else {
        hall_status &= 0xFE;
    }
    return hall_status;
}

/**
 * @brief 控制电机
 *
 * @param set_nums  绝对值是占空比  正负表示正反转
 */
void Int_BLDC_Control(int16_t set_nums)
{
    if (set_nums >= 0) {
        bldc_dir  = 0;
        dutyCycle = set_nums;
    } else {
        bldc_dir  = 1;
        dutyCycle = -set_nums;
    }
}

/**
 * @brief 定时器溢出的回调函数
 *
 * @param htim
 */
// void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
// {
//     if (htim->Instance == TIM8 && bldc_status == 1) {
//         // 电机正在运行  =>  需要处理换向
//         // 1. 获取霍尔状态
//         uint8_t hall_status = Int_BLDC_GetHall();
//         // 2. 根据霍尔状态的值 => 设置线圈情况
//         if (hall_status > 0 && hall_status < 7) {
//             switch (hall_status) {
//                 case 1:
//                     switch (bldc_dir) {
//                         case 0:
//                             // WH_VL
//                             __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_1, 0);
//                             __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_2, 0);
//                             __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_3, dutyCycle);
//                             HAL_GPIO_WritePin(U_L_GPIO_Port, U_L_Pin, GPIO_PIN_RESET);
//                             HAL_GPIO_WritePin(V_L_GPIO_Port, V_L_Pin, GPIO_PIN_SET);
//                             HAL_GPIO_WritePin(W_L_GPIO_Port, W_L_Pin, GPIO_PIN_RESET);
//                             break;
//                         case 1:
//                             // WL_VH
//                             __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_1, 0);
//                             __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_2, dutyCycle);
//                             __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_3, 0);
//                             HAL_GPIO_WritePin(U_L_GPIO_Port, U_L_Pin, GPIO_PIN_RESET);
//                             HAL_GPIO_WritePin(V_L_GPIO_Port, V_L_Pin, GPIO_PIN_RESET);
//                             HAL_GPIO_WritePin(W_L_GPIO_Port, W_L_Pin, GPIO_PIN_SET);
//                             break;
//                     }
//                     break;
//                 case 2:
//                     switch (bldc_dir) {
//                         case 0:
//                             // VH_UL
//                             __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_1, 0);
//                             __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_2, dutyCycle);
//                             __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_3, 0);
//                             HAL_GPIO_WritePin(U_L_GPIO_Port, U_L_Pin, GPIO_PIN_SET);
//                             HAL_GPIO_WritePin(V_L_GPIO_Port, V_L_Pin, GPIO_PIN_RESET);
//                             HAL_GPIO_WritePin(W_L_GPIO_Port, W_L_Pin, GPIO_PIN_RESET);
//                             break;
//                         case 1:
//                             // VL_UH
//                             __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_1, dutyCycle);
//                             __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_2, 0);
//                             __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_3, 0);
//                             HAL_GPIO_WritePin(U_L_GPIO_Port, U_L_Pin, GPIO_PIN_RESET);
//                             HAL_GPIO_WritePin(V_L_GPIO_Port, V_L_Pin, GPIO_PIN_SET);
//                             HAL_GPIO_WritePin(W_L_GPIO_Port, W_L_Pin, GPIO_PIN_RESET);
//                             break;
//                     }
//                     break;
//                 case 3:
//                     switch (bldc_dir) {
//                         case 0:
//                             // WH_UL
//                             __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_1, 0);
//                             __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_2, 0);
//                             __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_3, dutyCycle);
//                             HAL_GPIO_WritePin(U_L_GPIO_Port, U_L_Pin, GPIO_PIN_SET);
//                             HAL_GPIO_WritePin(V_L_GPIO_Port, V_L_Pin, GPIO_PIN_RESET);
//                             HAL_GPIO_WritePin(W_L_GPIO_Port, W_L_Pin, GPIO_PIN_RESET);
//                             break;
//                         case 1:
//                             // WL_UH
//                             __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_1, dutyCycle);
//                             __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_2, 0);
//                             __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_3, 0);
//                             HAL_GPIO_WritePin(U_L_GPIO_Port, U_L_Pin, GPIO_PIN_RESET);
//                             HAL_GPIO_WritePin(V_L_GPIO_Port, V_L_Pin, GPIO_PIN_RESET);
//                             HAL_GPIO_WritePin(W_L_GPIO_Port, W_L_Pin, GPIO_PIN_SET);
//                             break;
//                     }
//                     break;
//                 case 4:
//                     switch (bldc_dir) {
//                         case 0:
//                             // UH_WL
//                             __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_1, dutyCycle);
//                             __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_2, 0);
//                             __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_3, 0);
//                             HAL_GPIO_WritePin(U_L_GPIO_Port, U_L_Pin, GPIO_PIN_RESET);
//                             HAL_GPIO_WritePin(V_L_GPIO_Port, V_L_Pin, GPIO_PIN_RESET);
//                             HAL_GPIO_WritePin(W_L_GPIO_Port, W_L_Pin, GPIO_PIN_SET);
//                             break;
//                         case 1:
//                             // UL_WH
//                             __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_1, 0);
//                             __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_2, 0);
//                             __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_3, dutyCycle);
//                             HAL_GPIO_WritePin(U_L_GPIO_Port, U_L_Pin, GPIO_PIN_SET);
//                             HAL_GPIO_WritePin(V_L_GPIO_Port, V_L_Pin, GPIO_PIN_RESET);
//                             HAL_GPIO_WritePin(W_L_GPIO_Port, W_L_Pin, GPIO_PIN_RESET);
//                             break;
//                     }
//                     break;
//                 case 5:
//                     switch (bldc_dir) {
//                         case 0:
//                             // UH_VL
//                             __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_1, dutyCycle);
//                             __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_2, 0);
//                             __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_3, 0);
//                             HAL_GPIO_WritePin(U_L_GPIO_Port, U_L_Pin, GPIO_PIN_RESET);
//                             HAL_GPIO_WritePin(V_L_GPIO_Port, V_L_Pin, GPIO_PIN_SET);
//                             HAL_GPIO_WritePin(W_L_GPIO_Port, W_L_Pin, GPIO_PIN_RESET);
//                             break;
//                         case 1:
//                             // UL_VH
//                             __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_1, 0);
//                             __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_2, dutyCycle);
//                             __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_3, 0);
//                             HAL_GPIO_WritePin(U_L_GPIO_Port, U_L_Pin, GPIO_PIN_SET);
//                             HAL_GPIO_WritePin(V_L_GPIO_Port, V_L_Pin, GPIO_PIN_RESET);
//                             HAL_GPIO_WritePin(W_L_GPIO_Port, W_L_Pin, GPIO_PIN_RESET);
//                             break;
//                     }
//                     break;
//                 case 6:
//                     switch (bldc_dir) {
//                         case 0:
//                             // VH_WL
//                             __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_1, 0);
//                             __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_2, dutyCycle);
//                             __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_3, 0);
//                             HAL_GPIO_WritePin(U_L_GPIO_Port, U_L_Pin, GPIO_PIN_RESET);
//                             HAL_GPIO_WritePin(V_L_GPIO_Port, V_L_Pin, GPIO_PIN_RESET);
//                             HAL_GPIO_WritePin(W_L_GPIO_Port, W_L_Pin, GPIO_PIN_SET);
//                             break;
//                         case 1:
//                             // VL_WH
//                             __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_1, 0);
//                             __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_2, 0);
//                             __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_3, dutyCycle);
//                             HAL_GPIO_WritePin(U_L_GPIO_Port, U_L_Pin, GPIO_PIN_RESET);
//                             HAL_GPIO_WritePin(V_L_GPIO_Port, V_L_Pin, GPIO_PIN_SET);
//                             HAL_GPIO_WritePin(W_L_GPIO_Port, W_L_Pin, GPIO_PIN_RESET);
//                             break;
//                     }
//                     break;
//             }
//         }
//     }
// }

typedef struct {
    uint16_t *compareValues[3]; // CH1, CH2, CH3
    GPIO_PinState pinStates[3]; // U_L, V_L, W_L
} BldcPhaseConfig;

uint16_t zero = 0;
// 霍尔状态 + 方向 => 配置表
BldcPhaseConfig phaseTable[2][7] = {
    {
        // 正转方向 bldc_dir == 0
        {0},                                                                          // 保留 index 0
        {{&zero, &zero, &dutyCycle}, {GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_RESET}}, // case 1     // W_H V_L
        {{&zero, &dutyCycle, &zero}, {GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_RESET}}, // case 2     // V_H U_L
        {{&zero, &zero, &dutyCycle}, {GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_RESET}}, // case 3     // W_H U_L
        {{&dutyCycle, &zero, &zero}, {GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_SET}}, // case 4     // U_H W_L
        {{&dutyCycle, &zero, &zero}, {GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_RESET}}, // case 5     // U_H V_L
        {{&zero, &dutyCycle, &zero}, {GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_SET}}  // case 6     // V_H W_L
    },
    {
        // 反转方向 bldc_dir == 1
        {0},
        {{&zero, &dutyCycle, &zero}, {GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_SET}}, // case 1     // V_H U_L
        {{&dutyCycle, &zero, &zero}, {GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_RESET}}, // case 2     // U_H V_L
        {{&dutyCycle, &zero, &zero}, {GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_SET}}, // case 3     // U_H W_L
        {{&zero, &zero, &dutyCycle}, {GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_RESET}}, // case 4     // W_H U_L
        {{&zero, &dutyCycle, &zero}, {GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_RESET}}, // case 5     // V_H U_L
        {{&zero, &zero, &dutyCycle}, {GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_RESET}}  // case 6     // W_L V_H
    }};

static void Int_BLDC_Process(uint8_t hall_status)
{
    BldcPhaseConfig *config = &phaseTable[bldc_dir][hall_status];

    __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_1, *config->compareValues[0]);
    __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_2, *config->compareValues[1]);
    __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_3, *config->compareValues[2]);

    HAL_GPIO_WritePin(U_L_GPIO_Port, U_L_Pin, config->pinStates[0]);
    HAL_GPIO_WritePin(V_L_GPIO_Port, V_L_Pin, config->pinStates[1]);
    HAL_GPIO_WritePin(W_L_GPIO_Port, W_L_Pin, config->pinStates[2]);
}
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM8 && bldc_status == 1) {
        hall_count++;
        // 获取霍尔状态
        current_hall_status = Int_BLDC_GetHall();

        if (current_hall_status != last_hall_status) {

            // 霍尔状态发生变化
            step_count++;
            if (step_count == 12) {
                // 捕获最终计数
                hall_count_final = hall_count;
                // 复位计数值
                hall_count = 0;
                step_count = 0;
            }
            last_hall_status = current_hall_status;
            if (current_hall_status > 0 && current_hall_status < 7) {
                Int_BLDC_Process(current_hall_status);
            }
        }
    }
}
