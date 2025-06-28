#ifndef __INT_EEPROM__
#define __INT_EEPROM__

#include "i2c.h"

#define EEPROM_I2C_ADDR 0xA0

/**
 * @brief 写一个字节
 * 总共是256字节 => 1页16字节  单次写入最多16字节 继续写入会从页开始的位置覆盖
 * 写入数据之后需要等待5ms 才能读
 * @param addr
 * @param data
 */
void Int_EEPROM_Write(uint8_t addr, uint8_t data);

/**
 * @brief 读取一个字节
 *
 * @param addr
 */
uint8_t Int_EEPROM_Read(uint8_t addr);

#endif // __INT_EEPROM__
