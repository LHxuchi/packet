//
// Created by hyh on 2025/10/2.
//

#include "../../include/header/file_header.h"
#include "../../include/utils/checksum.h"

#include <chrono>
#include <cstring>

namespace data_packet
{
    std::unique_ptr<char[]> file_header::get_buffer()
    {
        std::unique_ptr<byte[]> buffer = std::make_unique<byte[]>(header_size());

        unsigned int offset = 0;
        auto append_in_buffer = [&buffer, &offset](const byte data[],unsigned short size)
        {
            for (unsigned short i = 0; i < size; i++)
            {
                buffer[offset + i] = data[i];
            }
            offset += size;
        };

        append_in_buffer(version, sizeof(version));
        append_in_buffer(creation_time, sizeof(creation_time));
        append_in_buffer(file_number, sizeof(file_number));
        append_in_buffer(file_size, sizeof(file_size));
        append_in_buffer(original_file_size, sizeof(original_file_size));
        append_in_buffer(checksum, sizeof(checksum));
        append_in_buffer(crc_32, sizeof(crc_32));

        return buffer;
    }

    void file_header::set_buffer(const byte* data, size_t size)
    {
        if (data == nullptr)
        {
            throw std::invalid_argument("[file_header::set_buffer] data pointer is null");
        }

        unsigned int offset = 0;
        auto write_in_header = [&data,&offset](byte member[], size_t num)
        {
            memcpy(member, data + offset, num);
            offset += num;
        };

        write_in_header(version, sizeof(version));
        write_in_header(creation_time, sizeof(creation_time));
        write_in_header(file_number, sizeof(file_number));
        write_in_header(file_size, sizeof(file_size));
        write_in_header(original_file_size, sizeof(original_file_size));
        write_in_header(checksum, sizeof(checksum));
        write_in_header(crc_32, sizeof(crc_32));

    }

    size_t file_header::header_size()
    {
        return SIZE;
    }

    word file_header::get_version() const
    {
        return make_word(std::make_tuple(version[0],version[1]));
    }

    void file_header::set_version(word other_version)
    {
        two_byte bytes = to_bytes(other_version);
        this->version[0] = std::get<0>(bytes);
        this->version[1] = std::get<1>(bytes);
    }

    dword file_header::get_crc_32() const
    {
        return make_dword(std::make_tuple(crc_32[0], crc_32[1], crc_32[2], crc_32[3]));
    }

    void file_header::set_crc_32(dword other_crc_32)
    {
        four_byte bytes = to_bytes(other_crc_32);
        this->crc_32[0] = std::get<0>(bytes);
        this->crc_32[1] = std::get<1>(bytes);
        this->crc_32[2] = std::get<2>(bytes);
        this->crc_32[3] = std::get<3>(bytes);
    }

    qword file_header::get_creation_time() const
    {
        return make_qword(std::make_tuple(creation_time[0], creation_time[1],
                                     creation_time[2], creation_time[3],
                                     creation_time[4], creation_time[5],
                                     creation_time[6], creation_time[7]));
    }

    void file_header::refresh_creation_time()
    {
        // 获取当前Unix时间戳（自1970-01-01 00:00:00 UTC以来的秒数）
        auto now = std::chrono::system_clock::now();
        auto epoch = now.time_since_epoch();
        auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(epoch).count();

        // 转换为64位无符号整数（Unix时间戳通常用32位或64位表示）
        auto unix_timestamp = static_cast<qword>(timestamp);

        // 转换为字节数组存储
        eight_byte bytes = to_bytes(unix_timestamp);

        creation_time[0] = std::get<0>(bytes);
        creation_time[1] = std::get<1>(bytes);
        creation_time[2] = std::get<2>(bytes);
        creation_time[3] = std::get<3>(bytes);
        creation_time[4] = std::get<4>(bytes);
        creation_time[5] = std::get<5>(bytes);
        creation_time[6] = std::get<6>(bytes);
        creation_time[7] = std::get<7>(bytes);
    }

    dword file_header::get_file_number() const
    {
        return make_dword(std::make_tuple(file_number[0], file_number[1],
                                     file_number[2], file_number[3]));
    }

    void file_header::set_file_number(dword other_file_number)
    {
        four_byte bytes = to_bytes(other_file_number);
        this->file_number[0] = std::get<0>(bytes);
        this->file_number[1] = std::get<1>(bytes);
        this->file_number[2] = std::get<2>(bytes);
        this->file_number[3] = std::get<3>(bytes);
    }

    qword file_header::get_file_size() const
    {
        return make_qword(std::make_tuple(file_size[0], file_size[1],
                                     file_size[2], file_size[3],
                                     file_size[4], file_size[5],
                                     file_size[6], file_size[7]));
    }

    void file_header::set_file_size(qword other_file_size)
    {
        eight_byte bytes = to_bytes(other_file_size);
        this->file_size[0] = std::get<0>(bytes);
        this->file_size[1] = std::get<1>(bytes);
        this->file_size[2] = std::get<2>(bytes);
        this->file_size[3] = std::get<3>(bytes);
        this->file_size[4] = std::get<4>(bytes);
        this->file_size[5] = std::get<5>(bytes);
        this->file_size[6] = std::get<6>(bytes);
        this->file_size[7] = std::get<7>(bytes);
    }

    qword file_header::get_original_file_size() const
    {
        return make_qword(std::make_tuple(original_file_size[0], original_file_size[1],
                                     original_file_size[2], original_file_size[3],
                                     original_file_size[4], original_file_size[5],
                                     original_file_size[6], original_file_size[7]));
    }

    void file_header::set_original_file_size(qword other_original_file_size)
    {
        eight_byte bytes = to_bytes(other_original_file_size);
        this->original_file_size[0] = std::get<0>(bytes);
        this->original_file_size[1] = std::get<1>(bytes);
        this->original_file_size[2] = std::get<2>(bytes);
        this->original_file_size[3] = std::get<3>(bytes);
        this->original_file_size[4] = std::get<4>(bytes);
        this->original_file_size[5] = std::get<5>(bytes);
        this->original_file_size[6] = std::get<6>(bytes);
        this->original_file_size[7] = std::get<7>(bytes);
    }

    dword file_header::get_checksum() const
    {
        return make_dword(std::make_tuple(checksum[0], checksum[1],
                                     checksum[2], checksum[3]));
    }

    void file_header::refresh_checksum()
    {
        std::vector<std::pair<const char*,size_t>> datas{
            {version,sizeof(version)},{creation_time,sizeof(creation_time)},
            {file_number,sizeof(file_number)},{file_size,sizeof(file_size)},
            {original_file_size,sizeof(original_file_size)},{crc_32,sizeof(crc_32)}
        };

        four_byte bytes = to_bytes(calculate_checksum(datas));

        checksum[0] = std::get<0>(bytes);
        checksum[1] = std::get<1>(bytes);
        checksum[2] = std::get<2>(bytes);
        checksum[3] = std::get<3>(bytes);
    }
} // data_packet