//
// Created by hyh on 2025/10/8.
//

#include "../../include/local_packet/local_packet.h"

#include <grp.h>
#include <pwd.h>
#include <sys/stat.h>

#include "../../include/utils/crc_32.h"


void data_packet::local_packet::refresh_crc_32()
{
    if (_data == nullptr)
    {
        if (_header -> get_file_size() != 0)
            throw std::runtime_error("[data_packet::local_packet::refresh_crc_32] called with nullptr");
        _header ->set_crc_32(0xffffffff);
        return;
    }

    _header->set_crc_32(CRC_calculate(reinterpret_cast<uint8_t*>(_data.get()), _header->get_file_size()));
}