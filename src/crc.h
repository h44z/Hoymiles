#ifndef __HM_crc_H__
#define __HM_crc_H__

#include <cstdint>

namespace Hoymiles
{
#define HM_CRC8_INIT 0x00
#define HM_CRC8_POLY 0x01

#define HM_CRC16_MODBUS_POLYNOM 0xA001
#define HM_CRC16_NRF24_POLYNOM 0x1021

uint8_t hm_crc8(uint8_t buf[], uint8_t len);
uint16_t hm_crc16(uint8_t buf[], uint8_t len, uint16_t start = 0xffff);
uint16_t hm_crc16nrf24(uint8_t buf[], uint16_t lenBits, uint16_t startBit = 0, uint16_t crcIn = 0xffff);
}

#endif // __HM_crc_H__