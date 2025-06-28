#ifndef __INT_EEPROM__
#define __INT_EEPROM__

#include "i2c.h"

#define EEPROM_I2C_ADDR 0xA0

/**
 * @brief дһ���ֽ�
 * �ܹ���256�ֽ� => 1ҳ16�ֽ�  ����д�����16�ֽ� ����д����ҳ��ʼ��λ�ø���
 * д������֮����Ҫ�ȴ�5ms ���ܶ�
 * @param addr
 * @param data
 */
void Int_EEPROM_Write(uint8_t addr, uint8_t data);

/**
 * @brief ��ȡһ���ֽ�
 *
 * @param addr
 */
uint8_t Int_EEPROM_Read(uint8_t addr);

#endif // __INT_EEPROM__
