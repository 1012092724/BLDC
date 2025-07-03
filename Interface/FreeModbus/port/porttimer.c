// /*
//  * FreeModbus Libary: BARE Port
//  * Copyright (C) 2006 Christian Walter <wolti@sil.at>
//  *
//  * This library is free software; you can redistribute it and/or
//  * modify it under the terms of the GNU Lesser General Public
//  * License as published by the Free Software Foundation; either
//  * version 2.1 of the License, or (at your option) any later version.
//  *
//  * This library is distributed in the hope that it will be useful,
//  * but WITHOUT ANY WARRANTY; without even the implied warranty of
//  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  * Lesser General Public License for more details.
//  *
//  * You should have received a copy of the GNU Lesser General Public
//  * License along with this library; if not, write to the Free Software
//  * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//  *
//  * File: $Id$
//  */

/* ------------------ Platform includes ------------------------*/
#include "port.h"
#include "tim.h"
/* ----------------- Modbus includes ---------------------------*/
#include "mb.h"
#include "mbport.h"

/* ------------------ static functions -------------------------*/
static void prvvTIMERExpiredISR(void);

/* ----------------- Start implementation ----------------------*/
BOOL xMBPortTimersInit(USHORT usTim1Timerout50us)
{
    // TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    // TIM_MasterConfigTypeDef sMasterConfig     = {0};

    // htim7.Instance               = TIM7;
    // htim7.Init.Prescaler         = 3599; // 50us记一次数
    // htim7.Init.CounterMode       = TIM_COUNTERMODE_UP;
    // htim7.Init.Period            = usTim1Timerout50us - 1; // usTim1Timerout50us(35) * 50us 即为定时器溢出时间
    // htim7.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    // htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    // if (HAL_TIM_Base_Init(&htim7) != HAL_OK) {
    //     return FALSE;
    // }
    // sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    // if (HAL_TIM_ConfigClockSource(&htim7, &sClockSourceConfig) != HAL_OK) {
    //     return FALSE;
    // }
    // sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    // sMasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;
    // if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK) {
    //     return FALSE;
    // }

    // __HAL_TIM_CLEAR_FLAG(&htim7, TIM_FLAG_UPDATE); // 先清除一下定时器的中断标记,防止使能中断后直接触发中断

    // __HAL_TIM_ENABLE_IT(&htim7, TIM_IT_UPDATE); // 使能定时器更新中断

    __HAL_TIM_CLEAR_FLAG(&htim7, TIM_FLAG_UPDATE);

    return TRUE;
}

/**
 * @brief 启用定时器
 *
 * 该函数启用之前通过xMBPortTimersInit()函数初始化的定时器。它首先将计数器清零，
 * 然后启用定时器，使其开始计时。这是在HAL库中对STM32定时器进行操作的典型方式。
 */
inline void vMBPortTimersEnable()
{

    /* Enable the timer with the timeout passed to xMBPortTimersInit( ) */
    __HAL_TIM_SET_COUNTER(&htim7, 0); // 清空计数器
    __HAL_TIM_ENABLE(&htim7);
    HAL_TIM_Base_Start_IT(&htim7);
}

inline void vMBPortTimersDisable()
{
    /* Disable any pending timers. */
    __HAL_TIM_DISABLE(&htim7); // 禁能定时器
    HAL_TIM_Base_Stop_IT(&htim7);
}

/* Create an ISR which is called whenever the timer has expired. This function
 * must then call pxMBPortCBTimerExpired( ) to notify the protocol stack that
 * the timer has expired.
 */
static void prvvTIMERExpiredISR(void)
{
    (void)pxMBPortCBTimerExpired();
}

/// 定时器7中断服务程序
void TIM7_IRQHandler(void)
{
    // if (__HAL_TIM_GET_FLAG(&htim7, TIM_FLAG_UPDATE)) // 更新中断标记被置位
    // {
    //     __HAL_TIM_CLEAR_FLAG(&htim7, TIM_FLAG_UPDATE); // 清除中断标记
    //     prvvTIMERExpiredISR();                         // 通知modbus3.5个字符等待时间到
    // }

    if (__HAL_TIM_GET_FLAG(&htim7, TIM_FLAG_UPDATE)) {
        // 清空中断标志位
        __HAL_TIM_CLEAR_IT(&htim7, TIM_IT_UPDATE);
        prvvTIMERExpiredISR();
    }

    HAL_TIM_IRQHandler(&htim7);
}

//// ===============================================================================================

// /*
//  * FreeModbus Libary: BARE Port
//  * Copyright (C) 2006 Christian Walter <wolti@sil.at>
//  *
//  * This library is free software; you can redistribute it and/or
//  * modify it under the terms of the GNU Lesser General Public
//  * License as published by the Free Software Foundation; either
//  * version 2.1 of the License, or (at your option) any later version.
//  *
//  * This library is distributed in the hope that it will be useful,
//  * but WITHOUT ANY WARRANTY; without even the implied warranty of
//  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  * Lesser General Public License for more details.
//  *
//  * You should have received a copy of the GNU Lesser General Public
//  * License along with this library; if not, write to the Free Software
//  * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//  *
//  * File: $Id$
//  */

// /* ----------------------- Platform includes --------------------------------*/
// #include "port.h"

// /* ----------------------- Modbus includes ----------------------------------*/
// #include "mb.h"
// #include "mbport.h"
// #include "tim.h"
// /* ----------------------- static functions ---------------------------------*/
// static void prvvTIMERExpiredISR(void);

// /* ----------------------- Start implementation -----------------------------*/
// BOOL xMBPortTimersInit(USHORT usTim1Timerout50us)
// {
//     // hal已经初始化定时器完成  35个50us
//     __HAL_TIM_CLEAR_FLAG(&htim7, TIM_FLAG_UPDATE);
//     return TRUE;
// }

// void vMBPortTimersEnable()
// {
//     /* Enable the timer with the timeout passed to xMBPortTimersInit( ) */
//     HAL_TIM_Base_Start_IT(&htim7);
// }

// void vMBPortTimersDisable()
// {
//     /* Disable any pending timers. */
//     // 清空计数值
//     __HAL_TIM_SET_COUNTER(&htim7, 0);
//     HAL_TIM_Base_Stop_IT(&htim7);
// }

// /* Create an ISR which is called whenever the timer has expired. This function
//  * must then call pxMBPortCBTimerExpired( ) to notify the protocol stack that
//  * the timer has expired.
//  */
// static void prvvTIMERExpiredISR(void)
// {
//     (void)pxMBPortCBTimerExpired();
// }

// void TIM7_IRQHandler(void)
// {
//     if (__HAL_TIM_GET_FLAG(&htim7, TIM_FLAG_UPDATE)) {
//         // 清空中断标志位
//         __HAL_TIM_CLEAR_IT(&htim7, TIM_IT_UPDATE);
//         prvvTIMERExpiredISR();
//     }

//     HAL_TIM_IRQHandler(&htim7);
// }
