//
// Created by hyh on 2025/10/3.
//

#ifndef DATA_BACK_UP_CHECKSUM_H
#define DATA_BACK_UP_CHECKSUM_H

#include <vector>
#include <cstdint>

namespace data_packet
{
    uint32_t calculate_checksum(const std::vector<std::pair<const char*,size_t>>& datas);
}

#endif //DATA_BACK_UP_CHECKSUM_H