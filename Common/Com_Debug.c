#include "Com_Debug.h"

void Com_Debug_Init(void)
{
  //MX_USART1_UART_Init();
}

int fputc(int _Ch,FILE *_File) {
    HAL_UART_Transmit(&huart1, (uint8_t *)&_Ch, 1, 1000);
    return _Ch;
}