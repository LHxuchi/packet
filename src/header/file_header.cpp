//
// Created by hyh on 2025/10/2.
//

#include "../../include/header/file_header.h"

namespace data_packet
{
    std::unique_ptr<char[]> file_header::get_buffer()
    {
        std::unique_ptr<byte[]> buffer = std::make_unique<byte[]>(header_size());

        auto write_in_buffer = [&buffer](const byte data[],unsigned int begin, unsigned int end)
        {
            unsigned int index = 0;
            while (begin <= end)
            {
                buffer[begin] = data[index++];
                ++begin;
            }
        };

        return buffer;
    }

    void file_header::set_buffer(const byte* data, size_t size)
    {
    }

    size_t file_header::header_size()
    {
        return SIZE;
    }
} // data_packet