/*
 * FreeModbus Libary: BARE Port
 * Copyright (C) 2006 Christian Walter <wolti@sil.at>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id$
 */
#include <stdio.h>
#include "port.h"
#include "usart.h"

/* ------------------- Modbus includes -----------------------*/
#include "mb.h"
#include "mbport.h"

/* ----------------- static functions -------------------------*/
static void prvvUARTTxReadyISR(void);

static void prvvUARTRxISR(void);

/* ---------------- Start implementation ---------------------*/
void vMBPortSerialEnable(BOOL xRxEnable, BOOL xTxEnable)
{
    /* If xRXEnable enable serial receive interrupts. If xTxENable enable
     * transmitter empty interrupts.
     */
    if (xRxEnable) {
        //
        // 如果使用了485控制芯片,那么在此处将485设置为接收模式
        //

        // MAX485_SetMode(MODE_RECV);

        __HAL_UART_ENABLE_IT(&huart4, UART_IT_RXNE); // 使能接收非空中断
    } else {
        __HAL_UART_DISABLE_IT(&huart4, UART_IT_RXNE); // 禁能接收非空中断
    }

    if (xTxEnable) {
        //
        // 如果使用了485控制芯片,那么在此处将485设置为发送模式
        //

        // MAX485_SetMode(MODE_SENT);

        __HAL_UART_ENABLE_IT(&huart4, UART_IT_TXE); // 使能发送为空中断
    } else {
        __HAL_UART_DISABLE_IT(&huart4, UART_IT_TXE); // 禁能发送为空中断
    }
}

/**
 * @brief 初始化串口通信参数
 *
 * 本函数用于初始化串口通信，包括波特率、数据位、奇偶校验等设置。
 *
 * @param ucPORT 串口端口号
 * @param ulBaudRate 波特率
 * @param ucDataBits 数据位数
 * @param eParity 校验方式
 * @return BOOL 初始化成功返回TRUE，失败返回FALSE
 */
BOOL xMBPortSerialInit(UCHAR ucPORT, ULONG ulBaudRate, UCHAR ucDataBits, eMBParity eParity)
{
    // 配置串口实例
    huart4.Instance = USART2;
    // 设置波特率
    huart4.Init.BaudRate = ulBaudRate;
    // 设置停止位
    huart4.Init.StopBits = UART_STOPBITS_1;
    // 设置模式为收发模式
    huart4.Init.Mode = UART_MODE_TX_RX;
    // 设置硬件流控制为无
    huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    // 设置过采样率
    huart4.Init.OverSampling = UART_OVERSAMPLING_16;

    // 根据校验方式配置串口参数
    switch (eParity) {
        // 奇校验
        case MB_PAR_ODD:
            // 设置奇校验
            huart4.Init.Parity = UART_PARITY_ODD;
            // 带奇偶校验数据位为9bits
            huart4.Init.WordLength = UART_WORDLENGTH_9B;
            break;

            // 偶校验
        case MB_PAR_EVEN:
            // 设置偶校验
            huart4.Init.Parity = UART_PARITY_EVEN;
            // 带奇偶校验数据位为9bits
            huart4.Init.WordLength = UART_WORDLENGTH_9B;
            break;

            // 无校验
        default:
            // 设置无校验
            huart4.Init.Parity = UART_PARITY_NONE;
            // 无奇偶校验数据位为8bits
            huart4.Init.WordLength = UART_WORDLENGTH_8B;
            break;
    }

    // 调用HAL库函数初始化串口，成功返回TRUE，失败返回FALSE
    return HAL_UART_Init(&huart4) == HAL_OK ? TRUE : FALSE;
}

/**
 * @brief 将一个字节写入UART的发送缓冲区。
 *
 * 该函数由协议栈调用，用于通过UART发送一个字节的数据。
 * 它在调用 `pxMBFrameCBTransmitterEmpty()` 后被调用，表示发送缓冲区已空，可以发送新的数据。
 *
 * @param ucByte 要发送的字节
 *
 * @return 返回 TRUE 表示操作成功
 */
BOOL xMBPortSerialPutByte(CHAR ucByte)
{
    /* 将字节写入USART2的数据寄存器，开始发送 */
    USART2->DR = ucByte;
    return TRUE;
}

/**
 * @brief 从串口接收缓冲区获取一个字节。
 *
 * 该函数在 pxMBFrameCBByteReceived() 被调用后由协议栈调用，用于获取 UART 接收缓冲区中的字节。
 *
 * @param pucByte 指向字符的指针，用于存储从 UART 接收缓冲区读取的字节。
 * @return 返回 TRUE 表示操作成功。
 */
BOOL xMBPortSerialGetByte(CHAR *pucByte)
{
    /* 从 USART1 的数据寄存器中读取低 8 位数据，并赋值给 pucByte 指向的变量 */
    *pucByte = (USART2->DR & (uint16_t)0x00FF);
    return TRUE;
}

/**
 * @brief 串行传输缓冲区空中断处理程序。
 *
 * 此函数作为目标处理器的串行传输缓冲区空中断处理程序。当中断触发时，
 * 该函数会调用 pxMBFrameCBTransmitterEmpty()，通知协议栈可以发送新字符。
 * 协议栈随后将调用 xMBPortSerialPutByte() 来发送字符。
 */
static void prvvUARTTxReadyISR(void)
{
    // 通知协议栈可以发送新字符
    pxMBFrameCBTransmitterEmpty();
}

/**
 * UART 接收中断服务例程 (ISR)。
 *
 * 此函数作为目标处理器的接收中断处理程序。
 * 它的作用是在接收到字符时调用 pxMBFrameCBByteReceived() 函数。
 * 协议栈随后会调用 xMBPortSerialGetByte() 来获取接收到的字符。
 */
static void prvvUARTRxISR(void)
{
    pxMBFrameCBByteReceived();
}

/**
 * @brief USART4中断服务例程
 *
 * 本函数处理USART4的中断请求，主要关注接收数据寄存器非空(RXNE)和发送数据寄存器为空(TXE)的中断标志。
 * 当接收到数据时，调用prvvUARTRxISR函数通知modbus有数据到达；
 * 当发送数据完成时，调用prvvUARTTxReadyISR函数通知modbus数据可以发送。
 */
void USART4_IRQHandler(void)
{
    // 检查接收数据寄存器非空标志位是否被置位
    if (__HAL_UART_GET_FLAG(&huart4, UART_FLAG_RXNE)) // 接收非空中断标记被置位
    {
        // 清除接收数据寄存器非空标志位
        __HAL_UART_CLEAR_FLAG(&huart4, UART_FLAG_RXNE); // 清除中断标记
        // 通知modbus有数据到达
        prvvUARTRxISR(); // 通知modbus有数据到达
    }

    // 检查发送数据寄存器为空标志位是否被置位
    if (__HAL_UART_GET_FLAG(&huart4, UART_FLAG_TXE)) // 发送为空中断标记被置位
    {
        // 清除发送数据寄存器为空标志位
        __HAL_UART_CLEAR_FLAG(&huart4, UART_FLAG_TXE); // 清除中断标记
        // 通知modbus数据可以发送
        prvvUARTTxReadyISR(); // 通知modbus数据可以发松
    }
}
