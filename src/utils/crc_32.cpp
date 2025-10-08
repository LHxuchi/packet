//
// Created by hyh on 2025/10/8.
//

#include "../../include/utils/crc_32.h"

uint32_t data_packet::CRC_calculate(const uint8_t* data, uint64_t size)
{
    return CRC_calculate(data, data+size);
}

bool data_packet::CRC_verify(uint32_t CRC_code, const uint8_t* data, uint64_t size)
{
    return CRC_calculate(data,size) == CRC_code;
}
