
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

#include "port.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"
#include "usart.h"
/* ----------------------- static functions ---------------------------------*/
static void prvvUARTTxReadyISR(void);
static void prvvUARTRxISR(void);

/* ----------------------- Start implementation -----------------------------*/
void vMBPortSerialEnable(BOOL xRxEnable, BOOL xTxEnable)
{
    /* If xRXEnable enable serial receive interrupts. If xTxENable enable
     * transmitter empty interrupts.
     */
    if (xRxEnable) {
        // 开启串口接收中断
        __HAL_UART_ENABLE_IT(&huart4, UART_IT_RXNE);
    } else {
        // 关闭串口接收中断
        __HAL_UART_DISABLE_IT(&huart4, UART_IT_RXNE);
    }

    if (xTxEnable) {
        // 开启串口发送中断
        __HAL_UART_ENABLE_IT(&huart4, UART_IT_TXE);
    } else {
        // 关闭串口发送中断
        __HAL_UART_DISABLE_IT(&huart4, UART_IT_TXE);
    }
}

BOOL xMBPortSerialInit(UCHAR ucPORT, ULONG ulBaudRate, UCHAR ucDataBits, eMBParity eParity)
{
    // HAL库已经实现了串口的初始化
    // 115200,  8,  NONE
    return TRUE;
}

BOOL xMBPortSerialPutByte(CHAR ucByte)
{
    /* Put a byte in the UARTs transmit buffer. This function is called
     * by the protocol stack if pxMBFrameCBTransmitterEmpty( ) has been
     * called. */
    UART4->DR = ucByte;
    return TRUE;
}

BOOL xMBPortSerialGetByte(CHAR *pucByte)
{
    /* Return the byte in the UARTs receive buffer. This function is called
     * by the protocol stack after pxMBFrameCBByteReceived( ) has been called.
     */
    *pucByte = UART4->DR;
    return TRUE;
}

/* Create an interrupt handler for the transmit buffer empty interrupt
 * (or an equivalent) for your target processor. This function should then
 * call pxMBFrameCBTransmitterEmpty( ) which tells the protocol stack that
 * a new character can be sent. The protocol stack will then call
 * xMBPortSerialPutByte( ) to send the character.
 */
static void prvvUARTTxReadyISR(void)
{
    pxMBFrameCBTransmitterEmpty();
}

/* Create an interrupt handler for the receive interrupt for your target
 * processor. This function should then call pxMBFrameCBByteReceived( ). The
 * protocol stack will then call xMBPortSerialGetByte( ) to retrieve the
 * character.
 */
static void prvvUARTRxISR(void)
{
    pxMBFrameCBByteReceived();
}

void UART4_IRQHandler(void)
{
    if (__HAL_UART_GET_FLAG(&huart4, UART_FLAG_RXNE) != RESET) {
        __HAL_UART_CLEAR_FLAG(&huart4, UART_FLAG_RXNE);
        prvvUARTRxISR();
    }
    if (__HAL_UART_GET_FLAG(&huart4, UART_FLAG_TXE) != RESET) {
        __HAL_UART_CLEAR_FLAG(&huart4, UART_FLAG_TXE);
        prvvUARTTxReadyISR();
    }

    HAL_UART_IRQHandler(&huart4);
}
