//
// Created by hyh on 2025/10/2.
//

#ifndef DATA_BACK_UP_FILE_HEADER_H
#define DATA_BACK_UP_FILE_HEADER_H

#include "header/header.h"
#include "../utils/byte_conversion.h"

namespace data_packet
{
    class file_header final : public header
    {
    public:
        file_header() = default;

        ~file_header() override = default;

        std::unique_ptr<char[]> get_buffer() override;

        void set_buffer(const char* data, size_t size) override;

        size_t header_size() override;

        [[nodiscard]] word get_version() const;

        void set_version(word other_version);

        [[nodiscard]] dword get_crc_32() const;

        void set_crc_32(dword other_crc_32);

        [[nodiscard]] qword get_creation_time() const;

        void refresh_creation_time();

        [[nodiscard]] dword get_file_number() const;

        void set_file_number(dword other_file_number);

        [[nodiscard]] qword get_file_size() const;

        void set_file_size(qword other_file_size);

        [[nodiscard]] qword get_original_file_size() const;

        void set_original_file_size(qword other_original_file_size);

        [[nodiscard]] dword get_checksum() const;

        void refresh_checksum();

    private:
        byte version[2]{0};
        byte creation_time[8]{0};
        byte file_number[4]{0};
        byte file_size[8]{0};
        byte original_file_size[8]{0};
        byte checksum[4]{0};
        byte crc_32[4]{0};

        static constexpr size_t SIZE = sizeof(version) +
            sizeof(creation_time) + sizeof(file_number) +
            sizeof(file_size) + sizeof(original_file_size) +
            sizeof(checksum) + sizeof(crc_32);
    };
} // data_packet

#endif //DATA_BACK_UP_FILE_HEADER_H