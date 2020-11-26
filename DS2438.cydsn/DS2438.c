/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "DS2438.h"
#include "DS2438_Commands.h"
#include "OneWire.h"
#include "project.h"

uint8_t DS2438_ComputeCrc(const uint8_t *data, uint8_t len);
DS2438_ErrorCode DS2438_CheckCrcValue(uint8_t* data, uint8_t len, uint8_t crc_value);

DS2438_ErrorCode DS2438_Init(void)
{
    return DS2438_OK;
}

DS2438_ErrorCode DS2438_DevIsPresent(void)
{
    if (OneWire_TouchReset(DS2438_Pin_0) == 0)
    {
        return DS2438_OK;
    }
    else
    {
        return DS2438_DEV_NOT_FOUND;
    }
}

DS2438_ErrorCode DS2438_ReadSerialNumber(uint8_t* serial_number, DS2438_CrcCheck check)
{
    uint8_t temp_rom[8];
    if (DS2438_ReadRawRom(temp_rom) == DS2438_OK)
    {
        if (check == DS2438_CRC_CHECK)
        {
            uint8_t crc_values[] = {temp_rom[0],temp_rom[1],temp_rom[2],
                                        temp_rom[3],temp_rom[4],temp_rom[5],temp_rom[6]};
            DS2438_ErrorCode error = DS2438_CheckCrcValue(crc_values, 7, temp_rom[7]);
            if (error != DS2438_OK)
                return DS2438_CRC_FAIL;
        }
        
        serial_number[0] = temp_rom[1];
        serial_number[1] = temp_rom[2];
        serial_number[2] = temp_rom[3];
        serial_number[3] = temp_rom[4];
        serial_number[4] = temp_rom[5];
        serial_number[5] = temp_rom[6];
        return DS2438_OK;
    }
    return DS2438_DEV_NOT_FOUND;
}

DS2438_ErrorCode DS2438_ReadRawRom(uint8_t* rom)
{
    // Reset sequence
    if (OneWire_TouchReset(DS2438_Pin_0) == 0)
    {
        // Write read rom command
        OneWire_WriteByte(DS2438_Pin_0, DS2438_READ_ROM);
        // Read 8 bytes of rom
        uint8_t loop;
        for (loop = 0; loop < 8; loop++)
        {
            rom[loop] = OneWire_ReadByte(DS2438_Pin_0);
        }
        return DS2438_OK;
    }
    return DS2438_DEV_NOT_FOUND;
}

DS2438_ErrorCode DS2438_StartVoltageConversion(void)
{
    // Reset sequence
    if (OneWire_TouchReset(DS2438_Pin_0) == 0)
    {
        // Write read rom command
        OneWire_WriteByte(DS2438_Pin_0, DS2438_SKIP_ROM);
        OneWire_WriteByte(DS2438_Pin_0, DS2438_VOLTAGE_CONV);
        return DS2438_OK;
    }
    
    return DS2438_DEV_NOT_FOUND;
    
}

DS2438_ErrorCode DS2438_StartTemperatureConversion(void)
{
    // Reset sequence
    if (OneWire_TouchReset(DS2438_Pin_0) == 0)
    {
        // Skip ROM and issue temperature conversion command
        OneWire_WriteByte(DS2438_Pin_0, DS2438_SKIP_ROM);
        OneWire_WriteByte(DS2438_Pin_0, DS2438_TEMP_CONV);
        return DS2438_OK;
    }
    
    return DS2438_DEV_NOT_FOUND;
}

DS2438_ErrorCode DS2438_HasTemperatureData(void)
{
    // Reset sequence
    if (OneWire_TouchReset(DS2438_Pin_0) == 0)
    {
        // Skip ROM command 
        OneWire_WriteByte(DS2438_Pin_0, DS2438_SKIP_ROM);
        // Recall memory command
        OneWire_WriteByte(DS2438_Pin_0, DS2438_RECALL_MEMORY);
        // Page 0
        OneWire_WriteByte(DS2438_Pin_0, 0x00);
        if (OneWire_TouchReset(DS2438_Pin_0) == 0)
        {
            // Skip ROM command 
            OneWire_WriteByte(DS2438_Pin_0, DS2438_SKIP_ROM);
            // Read scratchpad command
            OneWire_WriteByte(DS2438_Pin_0, DS2438_READ_SCRATCHPAD);
            // Page 0
            OneWire_WriteByte(DS2438_Pin_0, 0x00);
            // Read first byte
            uint8_t first_byte = OneWire_ReadByte(DS2438_Pin_0);
            if ((first_byte & (0x01 << 4)) == 0)
            {
                return DS2438_OK;
            }
            else
            {
                return DS2438_ERROR;
            }
            
        }
    }
    
    return DS2438_DEV_NOT_FOUND;
}

DS2438_ErrorCode DS2438_GetTemperatureData(float* temperature)
{
    // Reset sequence
    if (OneWire_TouchReset(DS2438_Pin_0) == 0)
    {
        // Skip ROM command 
        OneWire_WriteByte(DS2438_Pin_0, DS2438_SKIP_ROM);
        // Recall memory command
        OneWire_WriteByte(DS2438_Pin_0, DS2438_RECALL_MEMORY);
        // Page 0
        OneWire_WriteByte(DS2438_Pin_0, 0x00);
        if (OneWire_TouchReset(DS2438_Pin_0) == 0)
        {
            // Skip ROM command 
            OneWire_WriteByte(DS2438_Pin_0, DS2438_SKIP_ROM);
            // Read scratchpad command
            OneWire_WriteByte(DS2438_Pin_0, DS2438_READ_SCRATCHPAD);
            // Page 0
            OneWire_WriteByte(DS2438_Pin_0, 0x00);
            // Read nine bytes
            
            uint8_t page_data[9];
            if (DS2438_ReadPage(0x00, page_data) == DS2438_OK)
            {
                uint8_t temp_lsb = page_data[1];
                uint8_t temp_msb = page_data[2];
                *temperature = (((int16_t)temp_msb << 8) | ((temp_lsb & 0xFF) >> 3)) * 0.03125;
                return DS2438_OK;
            }
            else
            {
                return DS2438_ERROR;
            }
            
        }
    }
    
    return DS2438_DEV_NOT_FOUND;
}

DS2438_ErrorCode DS2438_HasVoltageData(void)
{
    // Reset sequence
    if (OneWire_TouchReset(DS2438_Pin_0) == 0)
    {
        // Skip ROM command 
        OneWire_WriteByte(DS2438_Pin_0, DS2438_SKIP_ROM);
        // Recall memory command
        OneWire_WriteByte(DS2438_Pin_0, DS2438_RECALL_MEMORY);
        // Page 0
        OneWire_WriteByte(DS2438_Pin_0, 0x00);
        if (OneWire_TouchReset(DS2438_Pin_0) == 0)
        {
            // Skip ROM command 
            OneWire_WriteByte(DS2438_Pin_0, DS2438_SKIP_ROM);
            // Read scratchpad command
            OneWire_WriteByte(DS2438_Pin_0, DS2438_READ_SCRATCHPAD);
            // Page 0
            OneWire_WriteByte(DS2438_Pin_0, 0x00);
            // Read first byte
            uint8_t first_byte = OneWire_ReadByte(DS2438_Pin_0);
            if ((first_byte & (0x01 << 6)) == 0)
            {
                return DS2438_OK;
            }
            else
            {
                return DS2438_ERROR;
            }
            
        }
    }
    
    return DS2438_DEV_NOT_FOUND;
}

DS2438_ErrorCode DS2438_GetVoltageData(float* voltage)
{
    // Reset sequence
    if (OneWire_TouchReset(DS2438_Pin_0) == 0)
    {
        // Skip ROM command 
        OneWire_WriteByte(DS2438_Pin_0, DS2438_SKIP_ROM);
        // Recall memory command
        OneWire_WriteByte(DS2438_Pin_0, DS2438_RECALL_MEMORY);
        // Page 0
        OneWire_WriteByte(DS2438_Pin_0, 0x00);
        if (OneWire_TouchReset(DS2438_Pin_0) == 0)
        {
            // Skip ROM command 
            OneWire_WriteByte(DS2438_Pin_0, DS2438_SKIP_ROM);
            // Read scratchpad command
            OneWire_WriteByte(DS2438_Pin_0, DS2438_READ_SCRATCHPAD);
            // Page 0
            OneWire_WriteByte(DS2438_Pin_0, 0x00);
            // Read nine bytes
            
            uint8_t page_data[9];
            if (DS2438_ReadPage(0x00, page_data) == DS2438_OK)
            {
                uint8_t volt_lsb = page_data[3];
                uint8_t volt_msb = page_data[4];
                *voltage = ((volt_msb << 8) | ((volt_lsb & 0xFF))) / 100.0;
                return DS2438_OK;
            }
            else
            {
                return DS2438_ERROR;
            }
            
        }
    }
    
    return DS2438_DEV_NOT_FOUND;
}

DS2438_ErrorCode DS2438_ReadPage(uint8_t page_number, uint8_t* page_data)
{
    if (page_number > 0x07)
        return DS2438_ERROR;
    else
    {
        // Reset sequence
        if (OneWire_TouchReset(DS2438_Pin_0) == 0)
        {
            // Skip ROM command 
            OneWire_WriteByte(DS2438_Pin_0, DS2438_SKIP_ROM);
            // Recall memory command
            OneWire_WriteByte(DS2438_Pin_0, DS2438_RECALL_MEMORY);
            // Page number
            OneWire_WriteByte(DS2438_Pin_0, page_number);
            if (OneWire_TouchReset(DS2438_Pin_0) == 0)
            {
                // Skip ROM command 
                OneWire_WriteByte(DS2438_Pin_0, DS2438_SKIP_ROM);
                // Read scratchpad command
                OneWire_WriteByte(DS2438_Pin_0, DS2438_READ_SCRATCHPAD);
                // Page 0
                OneWire_WriteByte(DS2438_Pin_0, page_number);
                // Read nine bytes
                for (uint8_t i = 0; i < 9; i++)
                {
                    page_data[i] = OneWire_ReadByte(DS2438_Pin_0);
                }
                return DS2438_OK;
                
            }
        }
    }
    return DS2438_ERROR;
}

DS2438_ErrorCode DS2438_WritePage(uint8_t page_number, uint8_t* page_data)
{
    if (page_number > 0x07)
        return DS2438_ERROR;
    else
    {
        // Reset sequence
        if (OneWire_TouchReset(DS2438_Pin_0) == 0)
        {
            // Skip ROM command 
            OneWire_WriteByte(DS2438_Pin_0, DS2438_SKIP_ROM);
            // Recall memory command
            OneWire_WriteByte(DS2438_Pin_0, DS2438_RECALL_MEMORY);
            // Page 0
            OneWire_WriteByte(DS2438_Pin_0, 0x00);
            if (OneWire_TouchReset(DS2438_Pin_0) == 0)
            {
                // Skip ROM command 
                OneWire_WriteByte(DS2438_Pin_0, DS2438_SKIP_ROM);
                // Read scratchpad command
                OneWire_WriteByte(DS2438_Pin_0, DS2438_READ_SCRATCHPAD);
                // Page 0
                OneWire_WriteByte(DS2438_Pin_0, page_number);
                // Read nine bytes
                for (uint8_t i = 0; i < 9; i++)
                {
                    page_data[i] = OneWire_ReadByte(DS2438_Pin_0);
                }
                return DS2438_OK;
                
            }
        }
    }
    return DS2438_ERROR;
}


DS2438_ErrorCode DS2438_CheckCrcValue(uint8_t* data, uint8_t len, uint8_t crc_value)
{
    uint8_t computed_crc = DS2438_ComputeCrc(data, len);
    if (computed_crc == crc_value)
    {
        return DS2438_OK;
    }
    else
    {
        return DS2438_CRC_FAIL;
    }
}

uint8_t DS2438_ComputeCrc(const uint8_t *data, uint8_t len)
{
    // Compute crc according to Dallas specs
    uint8_t crc = 0xff;
    uint8_t byteCtr;
    for (byteCtr = 0; byteCtr < len; ++byteCtr) {
        crc ^= data[byteCtr];
        for (uint8_t bit = 8; bit > 0; --bit) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x31;
            }
            else {
                crc = (crc << 1);
            }
        }
    }
    return crc;
}
/* [] END OF FILE */