//
// Created by hyh on 2025/10/3.
//

#include "../include/utils/checksum.h"

uint32_t data_packet::calculate_checksum(const std::vector<std::pair<const char*, size_t>>& datas)
{
    uint32_t calculated_checksum = 0xffffffff;
    for (const auto& data : datas)
    {
        for (size_t i = 0; i < data.second; i++)
        {
            uint8_t tmp = data.first[i];
            calculated_checksum ^= (tmp << (8 * (i % 4) ) );
        }
    }
    return calculated_checksum;
}
