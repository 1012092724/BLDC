#include "Int_EEPROM.h"

/**
 * @brief 写入多个字节
 * 总共是256字节 => 1页16字节  单次写入最多16字节 继续写入会从页开始的位置覆盖
 * 写入数据之后需要等待5ms 才能读
 * @param addr
 * @param data
 * @param len
 */
void Int_EEPROM_Write(uint8_t addr, uint8_t data)
{
    HAL_I2C_Mem_Write(&hi2c2, EEPROM_I2C_ADDR, addr, I2C_MEMADD_SIZE_8BIT, &data, 1, 1000);
    // 延时5ms
    HAL_Delay(5);
}

/**
 * @brief 读取多个字节
 *
 * @param addr
 * @param data
 * @param len
 */
uint8_t Int_EEPROM_Read(uint8_t addr)
{
    uint8_t data;
    HAL_I2C_Mem_Read(&hi2c2, EEPROM_I2C_ADDR, addr, I2C_MEMADD_SIZE_8BIT, &data, 1, 1000);
    return data;
}
