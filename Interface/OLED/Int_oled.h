#ifndef __OLED_H
#define __OLED_H

#include "gpio.h"
#include "i2c.h"
#include "stdlib.h"

#define OLED_I2C_ADDRESS 0x78

#define OLED_CMD         0
#define OLED_DATA        1

void Int_OLED_ColorTurn(uint8_t i);
void Int_OLED_DisplayTurn(uint8_t i);
void Int_OLED_WR_Byte(uint8_t dat, uint8_t mode);
void Int_OLED_DisPlay_On(void);
void Int_OLED_DisPlay_Off(void);
void Int_OLED_Refresh(void);
void Int_OLED_Clear(void);

void Int_OLED_ShowChar(uint8_t x, uint8_t y, uint8_t chr, uint8_t size1, uint8_t mode);
void Int_OLED_ShowString(uint8_t x, uint8_t y, uint8_t *chr, uint8_t size1, uint8_t mode);
void Int_OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t size1, uint8_t mode);
void Int_OLED_ShowChinese(uint8_t x, uint8_t y, uint8_t num, uint8_t size1, uint8_t mode);
void Int_OLED_ScrollDisplay(uint8_t num, uint8_t space, uint8_t mode);
void Int_OLED_ShowPicture(uint8_t x, uint8_t y, uint8_t sizex, uint8_t sizey, uint8_t BMP[], uint8_t mode);
void Int_OLED_Init(void);

#endif
