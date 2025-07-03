#include <stdio.h>
#include "stdlib.h"
#include "string.h"

/* ------------------ Platform includes -----------------------------*/
#include "port.h"

/* ------------------- Modbus includes ------------------------------*/
#include "mb.h"
#include "mbrtu.h"
#include "mbframe.h"

#include "mbcrc.h"
#include "mbport.h"

/* ----------------------- Defines -----------------------------------*/
#define MB_SER_PDU_SIZE_MIN 4   /*!< 最小的Modbus RTU PDU长度 */
#define MB_SER_PDU_SIZE_MAX 256 /*!< 最大的Modbus RTU PDU长度 */
#define MB_SER_PDU_SIZE_CRC 2   /*!< PDU中CRC校验码的长度 */
#define MB_SER_PDU_ADDR_OFF 0   /*!< Ser-PDU中地址字段的偏移量 */
#define MB_SER_PDU_PDU_OFF  1   /*!< Ser-PDU中Modbus-PDU的偏移量 */

/* ----------------------- Type definitions --------------------------*/
/**
 * 定义接收状态枚举类型，用于管理Modbus RTU接收过程的状态
 */
typedef enum {
    STATE_RX_INIT, /*!< 接收初始化状态 */
    STATE_RX_IDLE, /*!< 接收空闲状态 */
    STATE_RX_RCV,  /*!< 正在接收数据状态 */
    STATE_RX_ERROR /*!< 接收错误状态 */
} eMBRcvState;

/**
 * 定义发送状态枚举类型，用于管理Modbus RTU发送过程的状态
 */
typedef enum {
    STATE_TX_IDLE, /*!< 发送空闲状态 */
    STATE_TX_XMIT  /*!< 正在发送数据状态 */
} eMBSndState;

/* --------------------- Static variables ----------------------------*/
static volatile eMBSndState eSndState;
static volatile eMBRcvState eRcvState;

volatile UCHAR ucRTUBuf[MB_SER_PDU_SIZE_MAX];

static volatile UCHAR *pucSndBufferCur;
static volatile USHORT usSndBufferCount;

static volatile USHORT usRcvBufferPos;
/* --------------------- Start implementation ------------------------*/

/**
 * @brief 初始化 Modbus RTU 模块
 *
 * 配置并初始化 Modbus RTU 模块，设置串口通信参数，并启动定时器 T35。
 *
 * @param ucSlaveAddress 从站地址
 * @param ucPort 串口号
 * @param ulBaudRate 波特率
 * @param eParity 校验位 (无校验、奇校验或偶校验)
 *
 * @return 返回初始化结果
 *         - MB_ENOERR 成功
 *         - MB_EPORTERR 端口初始化失败
 */
eMBErrorCode eMBRTUInit(UCHAR ucSlaveAddress, UCHAR ucPort, ULONG ulBaudRate, eMBParity eParity)
{
    eMBErrorCode eStatus = MB_ENOERR;
    ULONG usTimerT35_50us;

    (void)ucSlaveAddress;
    ENTER_CRITICAL_SECTION();

    // 初始化串口通信，配置 Modbus RTU 所需的波特率和数据格式
    if (xMBPortSerialInit(ucPort, ulBaudRate, 8, eParity) != TRUE) {
        eStatus = MB_EPORTERR;
    } else {
        // 计算 T35 定时器的计数值（单位：50us）
        if (ulBaudRate > 19200) {
            // 对于高波特率，使用固定值 35 * 50us
            usTimerT35_50us = 35;
        } else {
            // 对于低波特率，计算 3.5 字符时间
            usTimerT35_50us = (7UL * 220000UL) / (2UL * ulBaudRate);
        }

        // 初始化定时器
        if (xMBPortTimersInit((USHORT)usTimerT35_50us) != TRUE) {
            eStatus = MB_EPORTERR;
        }
    }

    EXIT_CRITICAL_SECTION();

    return eStatus;
}

/**
 * @brief 启动 Modbus RTU 模块
 *
 * 启动 Modbus RTU 模块，使能串口接收功能，并启动定时器。
 */
void eMBRTUStart(void)
{
    ENTER_CRITICAL_SECTION();

    /*
     * 设置接收状态为初始化状态，等待第一个字符的到来以触发接收过程。
     * 如果在 T35 时间内没有接收到字符，则进入空闲状态。
     * 这样可以确保在启动 Modbus RTU 模块之前不会误读取到无效数据。
     */
    eRcvState = STATE_RX_INIT;
    vMBPortSerialEnable(TRUE, FALSE);
    vMBPortTimersEnable();

    EXIT_CRITICAL_SECTION();
}

/**
 * @brief 停止 Modbus RTU 模块
 *
 * 停止 Modbus RTU 模块，禁用串口接收和发送功能，并停止定时器。
 */
void eMBRTUStop(void)
{
    ENTER_CRITICAL_SECTION();

    // 禁用串口接收和发送功能
    vMBPortSerialEnable(FALSE, FALSE);

    // 禁用定时器
    vMBPortTimersDisable();

    EXIT_CRITICAL_SECTION();
}

/**
 * @brief 接收 Modbus RTU 数据帧
 *
 * 处理接收到的 Modbus RTU 数据帧，验证 CRC 校验码，并提取出 Modbus PDU。
 *
 * @param pucRcvAddress 接收到的地址指针
 * @param pucFrame 接收到的 Modbus PDU 数据指针
 * @param pusLength 接收到的 Modbus PDU 数据长度指针
 *
 * @return 返回接收结果
 *         - MB_ENOERR 成功
 *         - MB_EIO 接收失败
 */
BOOL xFrameReceived = FALSE;
eMBErrorCode eMBRTUReceive(UCHAR *pucRcvAddress, UCHAR **pucFrame, USHORT *pusLength)
{
    eMBErrorCode eStatus = MB_ENOERR;

    ENTER_CRITICAL_SECTION();
    assert(usRcvBufferPos < MB_SER_PDU_SIZE_MAX);

    /* 打印接收到的数据 */
    printf("Received Data: ");
    printf("%d bytes: ", usRcvBufferPos);
    for (USHORT i = 0; i < usRcvBufferPos; i++) {
        printf("%02X ", ucRTUBuf[i]);
    }
    printf("\r\n");

    /* 验证 CRC 校验码 */
    if ((usRcvBufferPos >= MB_SER_PDU_SIZE_MIN) && (usMBCRC16((UCHAR *)ucRTUBuf, usRcvBufferPos) == 0)) {

        /* 提取出地址字段 */
        *pucRcvAddress = ucRTUBuf[MB_SER_PDU_ADDR_OFF];

        /* 计算 Modbus PDU 的长度 */
        *pusLength = (USHORT)(usRcvBufferPos - MB_SER_PDU_PDU_OFF - MB_SER_PDU_SIZE_CRC);

        /* 提取出 Modbus PDU 数据 */
        *pucFrame      = (UCHAR *)&ucRTUBuf[MB_SER_PDU_PDU_OFF];
        xFrameReceived = TRUE;
    } else {
        eStatus = MB_EIO;
    }

    EXIT_CRITICAL_SECTION();
    return eStatus;
}

/**
 * @brief 发送 Modbus RTU 数据帧
 *
 * 构建并发送 Modbus RTU 数据帧，包括 CRC 校验码。
 *
 * @param ucSlaveAddress 从站地址
 * @param pucFrame Modbus PDU 数据指针
 * @param usLength Modbus PDU 数据长度
 *
 * @return 返回发送结果
 *         - MB_ENOERR 成功
 *         - MB_EIO 发送失败
 */
eMBErrorCode eMBRTUSend(UCHAR ucSlaveAddress, const UCHAR *pucFrame, USHORT usLength)
{
    eMBErrorCode eStatus = MB_ENOERR;
    USHORT usCRC16;

    ENTER_CRITICAL_SECTION();

    if (eRcvState == STATE_RX_IDLE) {
        /* 构建 Modbus RTU 帧 */
        pucSndBufferCur  = (UCHAR *)pucFrame - 1;
        usSndBufferCount = 1;

        /* 添加地址字段 */
        pucSndBufferCur[MB_SER_PDU_ADDR_OFF] = ucSlaveAddress;
        usSndBufferCount += usLength;

        /* 计算并添加 CRC */
        usCRC16                      = usMBCRC16((UCHAR *)pucSndBufferCur, usSndBufferCount);
        ucRTUBuf[usSndBufferCount++] = (UCHAR)(usCRC16 & 0xFF);
        ucRTUBuf[usSndBufferCount++] = (UCHAR)(usCRC16 >> 8);

        /*---------- 详细数据打印 ----------*/
        // 确保缓冲区至少有地址和功能码（至少4字节：地址+功能码+CRC）
        if (usSndBufferCount >= 4) {
            UCHAR ucID       = ucRTUBuf[0];          // 从站地址
            UCHAR ucFuncCode = ucRTUBuf[1];          // 功能码
            int iDataLen     = usSndBufferCount - 4; // 数据长度 = 总长 - 地址(1) - 功能码(1) - CRC(2)

            printf("RTU Sent: ID=0x%02X, Func=0x%02X, DataLen=%d, Data=[", ucID, ucFuncCode, iDataLen);

            // 打印数据部分（跳过地址、功能码和CRC）
            for (int i = 2; i < 2 + iDataLen; i++) {
                printf("%02X", ucRTUBuf[i]);
                if (i < 2 + iDataLen - 1) printf(" ");
            }
            printf("]\n");
        } else {
            printf("RTU Sent: Invalid frame length (%d bytes)\n", usSndBufferCount);
        }
        /*---------- 打印结束 ------------*/

        /* 启动发送 */
        eSndState = STATE_TX_XMIT;
        vMBPortSerialEnable(FALSE, TRUE);
    } else {
        eStatus = MB_EIO;
    }
    EXIT_CRITICAL_SECTION();
    return eStatus;
}

/**
 * @brief Modbus RTU 接收状态机处理函数
 *
 * 处理 Modbus RTU 接收状态机，根据当前状态处理接收到的数据。
 *
 * @return 返回是否需要调度任务
 */
BOOL xMBRTUReceiveFSM(void)
{
    BOOL xTaskNeedSwitch = FALSE;
    UCHAR ucByte;

    // 确保当前处于发送空闲状态
    assert(eSndState == STATE_TX_IDLE);

    // 从串口读取一个字节
    (void)xMBPortSerialGetByte((CHAR *)&ucByte);

    switch (eRcvState) {
        case STATE_RX_INIT:
            // 启动定时器，等待第一个字符到来
            vMBPortTimersEnable();
            break;

        case STATE_RX_ERROR:
            // 清除错误状态，重新启动定时器
            vMBPortTimersEnable();
            break;

        case STATE_RX_IDLE:
            // 接收到第一个字符，开始记录数据
            usRcvBufferPos             = 0;
            ucRTUBuf[usRcvBufferPos++] = ucByte;
            eRcvState                  = STATE_RX_RCV;

            // 启动定时器，等待后续字符
            vMBPortTimersEnable();
            break;

        case STATE_RX_RCV:
            // 继续接收数据，直到缓冲区满或出现错误
            if (usRcvBufferPos < MB_SER_PDU_SIZE_MAX) {
                ucRTUBuf[usRcvBufferPos++] = ucByte;
            } else {
                eRcvState = STATE_RX_ERROR;
            }
            vMBPortTimersEnable();
            break;
    }
    return xTaskNeedSwitch;
}

/**
 * @brief Modbus RTU 发送状态机处理函数
 *
 * 处理 Modbus RTU 发送状态机，根据当前状态发送数据。
 *
 * @return 返回是否需要调度任务
 */
BOOL xMBRTUTransmitFSM(void)
{
    BOOL xNeedPoll = FALSE;

    // 确保当前处于接收空闲状态
    assert(eRcvState == STATE_RX_IDLE);

    switch (eSndState) {
        case STATE_TX_IDLE:
            // 启用串口接收功能
            vMBPortSerialEnable(TRUE, FALSE);
            break;

        case STATE_TX_XMIT:
            // 发送数据，直到所有数据发送完毕
            if (usSndBufferCount != 0) {
                // 发送当前字节
                xMBPortSerialPutByte((CHAR)*pucSndBufferCur);
                pucSndBufferCur++;
                usSndBufferCount--;
            } else {
                // 发送完成，通知事件并重置状态
                xNeedPoll = xMBPortEventPost(EV_FRAME_SENT);
                vMBPortSerialEnable(TRUE, FALSE);
                eSndState = STATE_TX_IDLE;
            }
            break;
    }

    return xNeedPoll;
}

/**
 * @brief 检查 T35 定时器是否超时
 *
 * 检查 T35 定时器是否超时，并根据当前状态执行相应操作。
 *
 * @return 返回是否需要调度任务
 */
BOOL xMBRTUTimerT35Expired(void)
{
    BOOL xNeedPoll = FALSE;

    switch (eRcvState) {
        case STATE_RX_INIT:
            // T35 定时器超时，表示初始化阶段完成
            xNeedPoll = xMBPortEventPost(EV_READY);
            break;

        case STATE_RX_RCV:
            // 接收到完整帧，通知事件
            xNeedPoll = xMBPortEventPost(EV_FRAME_RECEIVED);
            break;

        case STATE_RX_ERROR:
            // 接收过程中出现错误
            break;

        default:
            // 非法状态检查
            assert((eRcvState == STATE_RX_INIT) ||
                   (eRcvState == STATE_RX_RCV) || (eRcvState == STATE_RX_ERROR));
    }

    // 禁用定时器并重置接收状态为空闲
    vMBPortTimersDisable();
    eRcvState = STATE_RX_IDLE;

    return xNeedPoll;
}

// ============================================================================================

// /*
//  * FreeModbus Libary: A portable Modbus implementation for Modbus ASCII/RTU.
//  * Copyright (c) 2006-2018 Christian Walter <cwalter@embedded-solutions.at>
//  * All rights reserved.
//  *
//  * Redistribution and use in source and binary forms, with or without
//  * modification, are permitted provided that the following conditions
//  * are met:
//  * 1. Redistributions of source code must retain the above copyright
//  *    notice, this list of conditions and the following disclaimer.
//  * 2. Redistributions in binary form must reproduce the above copyright
//  *    notice, this list of conditions and the following disclaimer in the
//  *    documentation and/or other materials provided with the distribution.
//  * 3. The name of the author may not be used to endorse or promote products
//  *    derived from this software without specific prior written permission.
//  *
//  * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
//  * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
//  * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
//  * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
//  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
//  * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//  * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//  * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//  * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
//  * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//  *
//  */

// /* ----------------------- System includes ----------------------------------*/
// #include "stdlib.h"
// #include "string.h"

// /* ----------------------- Platform includes --------------------------------*/
// #include "port.h"

// /* ----------------------- Modbus includes ----------------------------------*/
// #include "mb.h"
// #include "mbrtu.h"
// #include "mbframe.h"

// #include "mbcrc.h"
// #include "mbport.h"

// #include "stdio.h"
// /* ----------------------- Defines ------------------------------------------*/
// #define MB_SER_PDU_SIZE_MIN 4   /*!< Minimum size of a Modbus RTU frame. */
// #define MB_SER_PDU_SIZE_MAX 256 /*!< Maximum size of a Modbus RTU frame. */
// #define MB_SER_PDU_SIZE_CRC 2   /*!< Size of CRC field in PDU. */
// #define MB_SER_PDU_ADDR_OFF 0   /*!< Offset of slave address in Ser-PDU. */
// #define MB_SER_PDU_PDU_OFF  1   /*!< Offset of Modbus-PDU in Ser-PDU. */

// /* ----------------------- Type definitions ---------------------------------*/
// typedef enum {
//     STATE_RX_INIT, /*!< Receiver is in initial state. */
//     STATE_RX_IDLE, /*!< Receiver is in idle state. */
//     STATE_RX_RCV,  /*!< Frame is beeing received. */
//     STATE_RX_ERROR /*!< If the frame is invalid. */
// } eMBRcvState;

// typedef enum {
//     STATE_TX_IDLE, /*!< Transmitter is in idle state. */
//     STATE_TX_XMIT  /*!< Transmitter is in transfer state. */
// } eMBSndState;

// /* ----------------------- Static variables ---------------------------------*/
// static volatile eMBSndState eSndState;
// static volatile eMBRcvState eRcvState;

// volatile UCHAR ucRTUBuf[MB_SER_PDU_SIZE_MAX];

// static volatile UCHAR *pucSndBufferCur;
// static volatile USHORT usSndBufferCount;

// static volatile USHORT usRcvBufferPos;

// /* ----------------------- Start implementation -----------------------------*/
// eMBErrorCode
// eMBRTUInit(UCHAR ucSlaveAddress, UCHAR ucPort, ULONG ulBaudRate, eMBParity eParity)
// {
//     eMBErrorCode eStatus = MB_ENOERR;
//     ULONG usTimerT35_50us;

//     (void)ucSlaveAddress;
//     ENTER_CRITICAL_SECTION();

//     /* Modbus RTU uses 8 Databits. */
//     if (xMBPortSerialInit(ucPort, ulBaudRate, 8, eParity) != TRUE) {
//         eStatus = MB_EPORTERR;
//     } else {
//         /* If baudrate > 19200 then we should use the fixed timer values
//          * t35 = 1750us. Otherwise t35 must be 3.5 times the character time.
//          */
//         if (ulBaudRate > 19200) {
//             usTimerT35_50us = 35; /* 1800us. */
//         } else {
//             /* The timer reload value for a character is given by:
//              *
//              * ChTimeValue = Ticks_per_1s / ( Baudrate / 11 )
//              *             = 11 * Ticks_per_1s / Baudrate
//              *             = 220000 / Baudrate
//              * The reload for t3.5 is 1.5 times this value and similary
//              * for t3.5.
//              */
//             usTimerT35_50us = (7UL * 220000UL) / (2UL * ulBaudRate);
//         }
//         if (xMBPortTimersInit((USHORT)usTimerT35_50us) != TRUE) {
//             eStatus = MB_EPORTERR;
//         }
//     }
//     EXIT_CRITICAL_SECTION();

//     return eStatus;
// }

// void eMBRTUStart(void)
// {
//     ENTER_CRITICAL_SECTION();
//     /* Initially the receiver is in the state STATE_RX_INIT. we start
//      * the timer and if no character is received within t3.5 we change
//      * to STATE_RX_IDLE. This makes sure that we delay startup of the
//      * modbus protocol stack until the bus is free.
//      */
//     eRcvState = STATE_RX_INIT;
//     vMBPortSerialEnable(TRUE, FALSE);
//     vMBPortTimersEnable();

//     EXIT_CRITICAL_SECTION();
// }

// void eMBRTUStop(void)
// {
//     ENTER_CRITICAL_SECTION();
//     vMBPortSerialEnable(FALSE, FALSE);
//     vMBPortTimersDisable();
//     EXIT_CRITICAL_SECTION();
// }

// eMBErrorCode
// eMBRTUReceive(UCHAR *pucRcvAddress, UCHAR **pucFrame, USHORT *pusLength)
// {
//     BOOL xFrameReceived  = FALSE;
//     eMBErrorCode eStatus = MB_ENOERR;

//     ENTER_CRITICAL_SECTION();
//     assert(usRcvBufferPos < MB_SER_PDU_SIZE_MAX);

//     // 打印接收到的命令
//     for (uint8_t i = 0; i < usRcvBufferPos; i++) {
//         printf("%02x ", ucRTUBuf[i]);
//     }
//     printf("\n");

//     /* Length and CRC check */
//     if ((usRcvBufferPos >= MB_SER_PDU_SIZE_MIN) && (usMBCRC16((UCHAR *)ucRTUBuf, usRcvBufferPos) == 0)) {
//         /* Save the address field. All frames are passed to the upper layed
//          * and the decision if a frame is used is done there.
//          */
//         *pucRcvAddress = ucRTUBuf[MB_SER_PDU_ADDR_OFF];

//         /* Total length of Modbus-PDU is Modbus-Serial-Line-PDU minus
//          * size of address field and CRC checksum.
//          */
//         *pusLength = (USHORT)(usRcvBufferPos - MB_SER_PDU_PDU_OFF - MB_SER_PDU_SIZE_CRC);

//         /* Return the start of the Modbus PDU to the caller. */
//         *pucFrame      = (UCHAR *)&ucRTUBuf[MB_SER_PDU_PDU_OFF];
//         xFrameReceived = TRUE;
//     } else {
//         eStatus = MB_EIO;
//     }

//     EXIT_CRITICAL_SECTION();
//     return eStatus;
// }

// eMBErrorCode
// eMBRTUSend(UCHAR ucSlaveAddress, const UCHAR *pucFrame, USHORT usLength)
// {
//     eMBErrorCode eStatus = MB_ENOERR;
//     USHORT usCRC16;

//     ENTER_CRITICAL_SECTION();

//     /* Check if the receiver is still in idle state. If not we where to
//      * slow with processing the received frame and the master sent another
//      * frame on the network. We have to abort sending the frame.
//      */
//     if (eRcvState == STATE_RX_IDLE) {
//         /* First byte before the Modbus-PDU is the slave address. */
//         pucSndBufferCur  = (UCHAR *)pucFrame - 1;
//         usSndBufferCount = 1;

//         /* Now copy the Modbus-PDU into the Modbus-Serial-Line-PDU. */
//         pucSndBufferCur[MB_SER_PDU_ADDR_OFF] = ucSlaveAddress;
//         usSndBufferCount += usLength;

//         /* Calculate CRC16 checksum for Modbus-Serial-Line-PDU. */
//         usCRC16                      = usMBCRC16((UCHAR *)pucSndBufferCur, usSndBufferCount);
//         ucRTUBuf[usSndBufferCount++] = (UCHAR)(usCRC16 & 0xFF);
//         ucRTUBuf[usSndBufferCount++] = (UCHAR)(usCRC16 >> 8);

//         /* Activate the transmitter. */
//         eSndState = STATE_TX_XMIT;
//         vMBPortSerialEnable(FALSE, TRUE);
//     } else {
//         eStatus = MB_EIO;
//     }
//     EXIT_CRITICAL_SECTION();
//     return eStatus;
// }

// BOOL xMBRTUReceiveFSM(void)
// {
//     BOOL xTaskNeedSwitch = FALSE;
//     UCHAR ucByte;

//     assert(eSndState == STATE_TX_IDLE);

//     /* Always read the character. */
//     (void)xMBPortSerialGetByte((CHAR *)&ucByte);

//     switch (eRcvState) {
//             /* If we have received a character in the init state we have to
//              * wait until the frame is finished.
//              */
//         case STATE_RX_INIT:
//             vMBPortTimersEnable();
//             break;

//             /* In the error state we wait until all characters in the
//              * damaged frame are transmitted.
//              */
//         case STATE_RX_ERROR:
//             vMBPortTimersEnable();
//             break;

//             /* In the idle state we wait for a new character. If a character
//              * is received the t1.5 and t3.5 timers are started and the
//              * receiver is in the state STATE_RX_RECEIVCE.
//              */
//         case STATE_RX_IDLE:
//             usRcvBufferPos             = 0;
//             ucRTUBuf[usRcvBufferPos++] = ucByte;
//             eRcvState                  = STATE_RX_RCV;

//             /* Enable t3.5 timers. */
//             vMBPortTimersEnable();
//             break;

//             /* We are currently receiving a frame. Reset the timer after
//              * every character received. If more than the maximum possible
//              * number of bytes in a modbus frame is received the frame is
//              * ignored.
//              */
//         case STATE_RX_RCV:
//             if (usRcvBufferPos < MB_SER_PDU_SIZE_MAX) {
//                 ucRTUBuf[usRcvBufferPos++] = ucByte;
//             } else {
//                 eRcvState = STATE_RX_ERROR;
//             }
//             vMBPortTimersEnable();
//             break;
//     }
//     return xTaskNeedSwitch;
// }

// BOOL xMBRTUTransmitFSM(void)
// {
//     BOOL xNeedPoll = FALSE;

//     assert(eRcvState == STATE_RX_IDLE);

//     switch (eSndState) {
//             /* We should not get a transmitter event if the transmitter is in
//              * idle state.  */
//         case STATE_TX_IDLE:
//             /* enable receiver/disable transmitter. */
//             vMBPortSerialEnable(TRUE, FALSE);
//             break;

//         case STATE_TX_XMIT:
//             /* check if we are finished. */
//             if (usSndBufferCount != 0) {
//                 xMBPortSerialPutByte((CHAR)*pucSndBufferCur);
//                 pucSndBufferCur++; /* next byte in sendbuffer. */
//                 usSndBufferCount--;
//             } else {
//                 xNeedPoll = xMBPortEventPost(EV_FRAME_SENT);
//                 /* Disable transmitter. This prevents another transmit buffer
//                  * empty interrupt. */
//                 vMBPortSerialEnable(TRUE, FALSE);
//                 eSndState = STATE_TX_IDLE;
//             }
//             break;
//     }

//     return xNeedPoll;
// }

// BOOL xMBRTUTimerT35Expired(void)
// {
//     BOOL xNeedPoll = FALSE;

//     switch (eRcvState) {
//             /* Timer t35 expired. Startup phase is finished. */
//         case STATE_RX_INIT:
//             xNeedPoll = xMBPortEventPost(EV_READY);
//             break;

//             /* A frame was received and t35 expired. Notify the listener that
//              * a new frame was received. */
//         case STATE_RX_RCV:
//             xNeedPoll = xMBPortEventPost(EV_FRAME_RECEIVED);
//             break;

//             /* An error occured while receiving the frame. */
//         case STATE_RX_ERROR:
//             break;

//             /* Function called in an illegal state. */
//         default:
//             assert((eRcvState == STATE_RX_INIT) ||
//                    (eRcvState == STATE_RX_RCV) || (eRcvState == STATE_RX_ERROR));
//     }

//     vMBPortTimersDisable();
//     eRcvState = STATE_RX_IDLE;

//     return xNeedPoll;
// }
