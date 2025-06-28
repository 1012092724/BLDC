#include "Int_EEPROM.h"

/**
 * @brief д�����ֽ�
 * �ܹ���256�ֽ� => 1ҳ16�ֽ�  ����д�����16�ֽ� ����д����ҳ��ʼ��λ�ø���
 * д������֮����Ҫ�ȴ�5ms ���ܶ�
 * @param addr
 * @param data
 * @param len
 */
void Int_EEPROM_Write(uint8_t addr, uint8_t data)
{
    HAL_I2C_Mem_Write(&hi2c2, EEPROM_I2C_ADDR, addr, I2C_MEMADD_SIZE_8BIT, &data, 1, 1000);
    // ��ʱ5ms
    HAL_Delay(5);
}

/**
 * @brief ��ȡ����ֽ�
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
