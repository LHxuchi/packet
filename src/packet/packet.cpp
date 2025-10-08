//
// Created by hyh on 2025/10/8.
//

#include "../../include/packet/packet.h"
#include "../../include/utils/byte_conversion.h"
#include "../../include/utils/crc_32.h"

void data_packet::packet::refresh_file_size()
{
    qword size = _header.header_size();
    for (const auto& local_packet : _packets)
    {
        size += local_packet.info().get_file_size();
        size += local_packet.info().header_size();
    }
    _header.set_file_size(size);
}

void data_packet::packet::refresh_original_file_size()
{
    qword size = _header.header_size();
    for (const auto& local_packet : _packets)
    {
        size += local_packet.info().get_original_file_size();
        size += local_packet.info().header_size();
    }
    _header.set_original_file_size(size);
}

void data_packet::packet::refresh_crc_32()
{
    auto buffer = std::make_unique<byte[]>(_packets.size() * 4);
    size_t offset = 0;

    for (const auto& local_packet : _packets)
    {
        auto bytes = to_bytes(local_packet.info().get_crc_32());
        buffer[offset++] = std::get<0>(bytes);
        buffer[offset++] = std::get<1>(bytes);
        buffer[offset++] = std::get<2>(bytes);
        buffer[offset++] = std::get<3>(bytes);
    }

    _header.set_crc_32(CRC_calculate(reinterpret_cast<uint8_t*>(buffer.get()),offset));
}
