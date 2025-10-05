//
// Created by hyh on 2025/10/3.
//

#ifndef DATA_BACK_UP_LOCAL_FILE_HEADER_H
#define DATA_BACK_UP_LOCAL_FILE_HEADER_H

#include <filesystem>

#include "header.h"
#include "../utils/byte_conversion.h"

namespace data_packet
{
    class local_file_header final : public header
    {
    public:

        enum class compression_method
        {
            None = 0, LZ77
        };

        enum class encryption_method
        {
            None = 0, my_method
        };


        ~local_file_header() override = default;

        std::unique_ptr<char[]> get_buffer() override;

        void set_buffer(const char* data, size_t size) override;

        size_t header_size() override;

        [[nodiscard]] uint32_t get_uid() const;
        void set_uid(uint32_t uid);

        [[nodiscard]] uint32_t get_gid() const;
        void set_gid(uint32_t gid);

        [[nodiscard]] std::string get_uname() const;
        void set_uname(const std::string& uname);

        [[nodiscard]] std::string get_gname() const;
        void set_gname(const std::string& gname);

        [[nodiscard]] uint64_t get_creation_time() const;
        void refresh_creation_time();

        [[nodiscard]] uint64_t get_last_modification_time() const;
        void set_last_modification_time(uint64_t last_modification_time);

        [[nodiscard]] uint64_t get_last_access_time() const;
        void set_last_access_time(uint64_t last_access_time);

        [[nodiscard]] std::filesystem::perms get_permissions() const;
        void set_permissions(const std::filesystem::perms& permissions);

        [[nodiscard]] std::filesystem::file_type get_file_type() const;
        void set_file_type(const std::filesystem::file_type& file_type);

        [[nodiscard]] uint32_t get_crc_32() const;
        void set_crc_32(uint32_t crc_32);

        [[nodiscard]] uint32_t get_checksum() const;
        void refresh_checksum();

        [[nodiscard]] compression_method get_compression_method() const;
        void set_compression_method(compression_method compression_method);

        [[nodiscard]] encryption_method get_encryption_method() const;
        void set_encryption_method(encryption_method encryption_method);

        [[nodiscard]] std::array<uint8_t, 16> get_salt() const;
        void set_salt(const std::array<uint8_t, 16>& salt);

        [[nodiscard]] uint16_t get_link_name_length() const;
        void set_link_name_length(uint16_t link_name_length);

        [[nodiscard]] uint64_t get_original_file_size() const;
        void set_original_file_size(uint64_t original_file_size);

        [[nodiscard]] uint64_t get_file_size() const;
        void set_file_size(uint64_t file_size);

        [[nodiscard]] uint16_t get_file_name_length() const;
        void set_file_name_length(uint16_t file_name_length);

        [[nodiscard]] std::string get_link_name() const;
        void set_link_name(const std::string& link_name);

        [[nodiscard]] std::string get_file_name() const;
        void set_file_name(const std::string& file_name);

    private:
        byte uid_[4]{0};
        byte gid_[4]{0};
        byte uname_[32]{0};
        byte gname_[32]{0};
        byte creation_time_[8]{0};
        byte last_modification_time_[8]{0};
        byte last_access_time_[8]{0};
        byte file_type_and_permissions_[2]{0};
        byte crc_32_[4]{0};
        byte checksum_[4]{0};
        byte compression_and_encryption_{0};
        byte salt_[16]{0};
        byte original_file_size_[8]{0};
        byte file_size_[8]{0};
        byte link_name_length_[2]{0};
        byte file_name_length_[2]{0};

        std::unique_ptr<byte[]> link_name_{nullptr};
        std::unique_ptr<byte[]> file_name_{nullptr};


        static constexpr byte compression_method_mask = 0xf0;
        static constexpr byte encryption_method_mask = 0xf;
        static constexpr word permission_mask = 0x1ff;
        static constexpr byte file_type_mask = 0xfe;

        static constexpr size_t SIZE = sizeof(uid_) + sizeof(gid_) + sizeof(uname_) +
            sizeof(gname_) + sizeof(creation_time_) + sizeof(last_modification_time_) +
                sizeof(last_access_time_) + sizeof(file_type_and_permissions_) + sizeof(crc_32_)+
                    sizeof(checksum_) + sizeof(compression_and_encryption_) + sizeof(salt_) +
                        sizeof(original_file_size_) + sizeof(file_size_) + sizeof(link_name_length_) +
                            + sizeof(file_name_length_);
    };
} // data_packet

#endif //DATA_BACK_UP_LOCAL_FILE_HEADER_H