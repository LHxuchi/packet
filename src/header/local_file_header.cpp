//
// Created by hyh on 2025/10/3.
//

#include <array>

#include "../../include/header/local_file_header.h"
#include "../../include/utils/byte_conversion.h"
#include "../../include/utils/checksum.h"

namespace data_packet
{
    std::unique_ptr<char[]> local_file_header::get_buffer() const
    {
        auto buffer = std::make_unique<byte[]>(header_size());

        unsigned int offset = 0;

        auto append_to_buffer = [&buffer,&offset](const byte* data,unsigned int size)
        {
            for (unsigned int i = 0; i < size; i++)
            {
                buffer[i + offset] = data[i];
            }
            offset += size;
        };

        append_to_buffer(uid_, sizeof(uid_));
        append_to_buffer(gid_, sizeof(gid_));
        append_to_buffer(uname_, sizeof(uname_));
        append_to_buffer(gname_, sizeof(gname_));
        append_to_buffer(creation_time_, sizeof(creation_time_));
        append_to_buffer(last_modification_time_, sizeof(last_modification_time_));
        append_to_buffer(last_access_time_, sizeof(last_access_time_));
        append_to_buffer(file_type_and_permissions_, sizeof(file_type_and_permissions_));
        append_to_buffer(crc_32_, sizeof(crc_32_));
        append_to_buffer(checksum_, sizeof(checksum_));
        append_to_buffer(&compression_and_encryption_, sizeof(compression_and_encryption_));
        append_to_buffer(salt_, sizeof(salt_));
        append_to_buffer(original_file_size_, sizeof(original_file_size_));
        append_to_buffer(file_size_, sizeof(file_size_));
        append_to_buffer(link_name_length_, sizeof(link_name_length_));
        append_to_buffer(file_name_length_, sizeof(file_name_length_));
        append_to_buffer(link_name_.get(), get_link_name_length());
        append_to_buffer(file_name_.get(), get_file_name_length());

        return buffer;
    }

    void local_file_header::set_buffer(const char* data)
    {
        if (!data)
        {
            throw std::invalid_argument("[local_file_header::set_buffer] data pointer is null");
        }


        unsigned int offset = 0;
        auto get_data = [&data,&offset](byte* posi,size_t len)
        {
            for (unsigned int i = 0; i < len; i++)
            {
                posi[i] = data[i + offset];
            }
            offset += len;
        };

        get_data(uid_, sizeof(uid_));
        get_data(gid_, sizeof(gid_));
        get_data(uname_, sizeof(uname_));
        get_data(gname_, sizeof(gname_));
        get_data(creation_time_, sizeof(creation_time_));
        get_data(last_modification_time_, sizeof(last_modification_time_));
        get_data(last_access_time_, sizeof(last_access_time_));
        get_data(file_type_and_permissions_, sizeof(file_type_and_permissions_));
        get_data(crc_32_, sizeof(crc_32_));
        get_data(checksum_, sizeof(checksum_));
        get_data(&compression_and_encryption_, sizeof(compression_and_encryption_));
        get_data(salt_, sizeof(salt_));
        get_data(original_file_size_, sizeof(original_file_size_));
        get_data(file_size_, sizeof(file_size_));
        get_data(link_name_length_, sizeof(link_name_length_));
        get_data(file_name_length_, sizeof(file_name_length_));
        link_name_ = std::make_unique<byte[]>(get_link_name_length());
        get_data(link_name_.get(), get_link_name_length());
        file_name_ = std::make_unique<byte[]>(get_file_name_length());
        get_data(file_name_.get(), get_file_name_length());

    }

    size_t local_file_header::header_size() const
    {
        return make_word({file_name_length_[0], file_name_length_[1]}) +
            make_word({link_name_length_[0], link_name_length_[1]}) + SIZE;
    }

    uint32_t local_file_header::get_uid() const
    {
        return make_dword({uid_[0], uid_[1], uid_[2], uid_[3]});
    }

    void local_file_header::set_uid(uint32_t uid)
    {
        const four_byte bytes = to_bytes(uid);
        uid_[0] = std::get<0>(bytes);
        uid_[1] = std::get<1>(bytes);
        uid_[2] = std::get<2>(bytes);
        uid_[3] = std::get<3>(bytes);
    }

    uint32_t local_file_header::get_gid() const
    {
        return make_dword({gid_[0],gid_[1],gid_[2],gid_[3]});
    }

    void local_file_header::set_gid(uint32_t gid)
    {
        const four_byte bytes = to_bytes(gid);
        gid_[0] = std::get<0>(bytes);
        gid_[1] = std::get<1>(bytes);
        gid_[2] = std::get<2>(bytes);
        gid_[3] = std::get<3>(bytes);
    }

    std::string local_file_header::get_uname() const
    {
        std::string ret;
        ret.reserve(32);
        for (unsigned int i = 0; i < 32; i++)
        {
            if (uname_[i] == '\0')
                break;
            ret.push_back(uname_[i]);
        }
        return ret;
    }

    void local_file_header::set_uname(const std::string& uname)
    {
        unsigned int size = uname.size() < 32 ?uname.size():32;
        for (unsigned int i = 0; i < size; i++)
        {
            uname_[i] = uname[i];
        }
    }

    std::string local_file_header::get_gname() const
    {
        std::string ret;
        ret.reserve(32);
        for (unsigned int i = 0; i < 32; i++)
        {
            if (gname_[i] == '\0')
                break;
            ret.push_back(gname_[i]);
        }
        return ret;
    }

    void local_file_header::set_gname(const std::string& gname)
    {
        unsigned int size = gname.size()<32 ?gname.size():32;
        for (unsigned int i = 0; i < size; i++)
        {
            gname_[i] = gname[i];
        }
    }

    uint64_t local_file_header::get_creation_time() const
    {
        return make_qword({creation_time_[0], creation_time_[1], creation_time_[2], creation_time_[3],
                      creation_time_[4], creation_time_[5], creation_time_[6], creation_time_[7]});
    }

    void local_file_header::refresh_creation_time()
    {
        // 获取当前Unix时间戳（自1970-01-01 00:00:00 UTC以来的秒数）
        auto now = std::chrono::system_clock::now();
        auto epoch = now.time_since_epoch();
        auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(epoch).count();

        // 转换为64位无符号整数（Unix时间戳通常用32位或64位表示）
        auto unix_timestamp = static_cast<qword>(timestamp);

        // 转换为字节数组存储
        eight_byte bytes = to_bytes(unix_timestamp);

        creation_time_[0] = std::get<0>(bytes);
        creation_time_[1] = std::get<1>(bytes);
        creation_time_[2] = std::get<2>(bytes);
        creation_time_[3] = std::get<3>(bytes);
        creation_time_[4] = std::get<4>(bytes);
        creation_time_[5] = std::get<5>(bytes);
        creation_time_[6] = std::get<6>(bytes);
        creation_time_[7] = std::get<7>(bytes);
    }

    uint64_t local_file_header::get_last_modification_time() const
    {
        return make_qword({last_modification_time_[0], last_modification_time_[1],
                      last_modification_time_[2], last_modification_time_[3],
                      last_modification_time_[4], last_modification_time_[5],
                      last_modification_time_[6], last_modification_time_[7]});
    }

    void local_file_header::set_last_modification_time(uint64_t last_modification_time)
    {
        const eight_byte bytes = to_bytes(last_modification_time);
        last_modification_time_[0] = std::get<0>(bytes);
        last_modification_time_[1] = std::get<1>(bytes);
        last_modification_time_[2] = std::get<2>(bytes);
        last_modification_time_[3] = std::get<3>(bytes);
        last_modification_time_[4] = std::get<4>(bytes);
        last_modification_time_[5] = std::get<5>(bytes);
        last_modification_time_[6] = std::get<6>(bytes);
        last_modification_time_[7] = std::get<7>(bytes);
    }

    uint64_t local_file_header::get_last_access_time() const
    {
        return make_qword({last_access_time_[0], last_access_time_[1],
                      last_access_time_[2], last_access_time_[3],
                      last_access_time_[4], last_access_time_[5],
                      last_access_time_[6], last_access_time_[7]});
    }

    void local_file_header::set_last_access_time(uint64_t last_access_time)
    {
        const eight_byte bytes = to_bytes(last_access_time);

        last_access_time_[0] = std::get<0>(bytes);
        last_access_time_[1] = std::get<1>(bytes);
        last_access_time_[2] = std::get<2>(bytes);
        last_access_time_[3] = std::get<3>(bytes);
        last_access_time_[4] = std::get<4>(bytes);
        last_access_time_[5] = std::get<5>(bytes);
        last_access_time_[6] = std::get<6>(bytes);
        last_access_time_[7] = std::get<7>(bytes);
    }

    std::filesystem::perms local_file_header::get_permissions() const
    {
        word perms_bits = make_word({file_type_and_permissions_[0],
            file_type_and_permissions_[1]}) & permission_mask;
        std::filesystem::perms permissions {perms_bits};
        return permissions;
    }

    void local_file_header::set_permissions(const std::filesystem::perms& permissions)
    {
        word perms_bits = static_cast<word>(permissions);
        perms_bits &= permission_mask;
        auto bytes = to_bytes(perms_bits);
        file_type_and_permissions_[1] = std::get<1>(bytes);

        file_type_and_permissions_[0] &= static_cast<byte>(0xfe);
        file_type_and_permissions_[0] |= static_cast<byte>(std::get<0>(bytes) & static_cast<byte>(0x01));
    }

    std::filesystem::file_type local_file_header::get_file_type() const
    {
        using fs_type = std::filesystem::file_type;
        word type_bits = file_type_and_permissions_[0] & file_type_mask;
        type_bits >>= 1;
        switch (type_bits)
        {
        case 0:
            return fs_type::none;
        case 1:
            return fs_type::regular;
        case 2:
            return fs_type::directory;
        case 3:
            return fs_type::symlink;
        case 4:
            return fs_type::block;
        case 5:
            return fs_type::character;
        case 6:
            return fs_type::fifo;
        case 7:
            return fs_type::socket;
        default:
            return fs_type::unknown;
        }
    }

    void local_file_header::set_file_type(const std::filesystem::file_type& file_type)
    {
        using fs_type = std::filesystem::file_type;
        byte type_bits; // 默认对应fs_type::unknown

        // 根据文件类型确定对应的type_bits值
        switch (file_type)
        {
        case fs_type::none:
            type_bits = 0;
            break;
        case fs_type::regular:
            type_bits = 1;
            break;
        case fs_type::directory:
            type_bits = 2;
            break;
        case fs_type::symlink:
            type_bits = 3;
            break;
        case fs_type::block:
            type_bits = 4;
            break;
        case fs_type::character:
            type_bits = 5;
            break;
        case fs_type::fifo:
            type_bits = 6;
            break;
        case fs_type::socket:
            type_bits = 7;
            break;
        default: // 包括fs_type::unknown及其他未定义类型
            type_bits = (byte)0xff; // 使用一个超出已知范围的值表示未知类型
            break;
        }

        // 清除原有类型位，然后设置新的类型位
        // 先将type_bits左移9位，对应get_file_type中右移9位的逆操作
        // 再用file_type_mask掩码确保只修改类型相关的位
        file_type_and_permissions_[0] &= 1;
        file_type_and_permissions_[0] |= static_cast<byte>(type_bits << 1);
    }

    uint32_t local_file_header::get_crc_32() const
    {
        return make_dword({crc_32_[0], crc_32_[1], crc_32_[2], crc_32_[3]});
    }

    void local_file_header::set_crc_32(uint32_t crc_32)
    {
        const four_byte bytes = to_bytes(crc_32);
        crc_32_[0] = std::get<0>(bytes);
        crc_32_[1] = std::get<1>(bytes);
        crc_32_[2] = std::get<2>(bytes);
        crc_32_[3] = std::get<3>(bytes);
    }

    uint32_t local_file_header::get_checksum() const
    {
        return make_dword({checksum_[0], checksum_[1], checksum_[2], checksum_[3]});
    }

    void local_file_header::refresh_checksum()
    {
        const std::vector<std::pair<const byte*, size_t>> datas{
        {uid_, sizeof(uid_)},
        {gid_, sizeof(gid_)},
        {uname_, sizeof(uname_)},
        {gname_, sizeof(gname_)},
        {creation_time_, sizeof(creation_time_)},
        {last_modification_time_, sizeof(last_modification_time_)},
        {last_access_time_, sizeof(last_access_time_)},
        {file_type_and_permissions_, sizeof(file_type_and_permissions_)},
        {crc_32_, sizeof(crc_32_)},
        {&compression_and_encryption_, sizeof(compression_and_encryption_)},
        {salt_, sizeof(salt_)},
        {original_file_size_, sizeof(original_file_size_)},
        {file_size_, sizeof(file_size_)},
        {link_name_length_, sizeof(link_name_length_)},
        {file_name_length_, sizeof(file_name_length_)},
        {link_name_.get(),get_link_name_length()},
        {file_name_.get(),get_file_name_length()}
        };

        auto bytes = to_bytes(calculate_checksum(datas));
        checksum_[0] = std::get<0>(bytes);
        checksum_[1] = std::get<1>(bytes);
        checksum_[2] = std::get<2>(bytes);
        checksum_[3] = std::get<3>(bytes);
    }

    local_file_header::compression_method local_file_header::get_compression_method() const
    {
        byte compression_method_bits = compression_and_encryption_ & compression_method_mask;
        compression_method_bits >>= 4;
        switch (compression_method_bits)
        {
        case 0:
            return compression_method::None;
        case 1:
            return compression_method::LZ77;
        case 2:
            return compression_method::HUFFMAN;
        default:
            return compression_method::None;
        }

    }

    void local_file_header::set_compression_method(compression_method compression_method)
    {
        byte compression_method_bits = 0;
        switch (compression_method)
        {
        case compression_method::None:
            compression_method_bits = 0;
            break;
        case compression_method::LZ77:
            compression_method_bits = 1;
            break;
        case compression_method::HUFFMAN:
            compression_method_bits = 2;
            break;
        default:
            compression_method_bits = 0;
        }
        compression_method_bits <<= 4;
        compression_and_encryption_ &= ~compression_method_mask;
        compression_and_encryption_ |= compression_method_bits;
    }

    local_file_header::encryption_method local_file_header::get_encryption_method() const
    {
        // 提取低4位（加密方法位）
        byte encryption_method_bits = compression_and_encryption_ & encryption_method_mask; // 0x0F是低4位的掩码

        switch (encryption_method_bits)
        {
        case 0:
            return encryption_method::None;
        case 1:
            return encryption_method::my_method;
        case 2:
            return encryption_method::AES_256_CBC;
        default:
            // 对于未知值，返回默认的None
            return encryption_method::None;
        }
    }

    void local_file_header::set_encryption_method(encryption_method encryption_method)
    {
        byte encryption_method_bits = 0;

        // 根据枚举值确定对应的位值
        switch (encryption_method)
        {
        case encryption_method::None:
            encryption_method_bits = 0;
            break;
        case encryption_method::my_method:
            encryption_method_bits = 1;
            break;
        case encryption_method::AES_256_CBC:
            encryption_method_bits = 2;
            break;
        default:
            encryption_method_bits = 0;
        }

        // 清除低4位的旧值，然后设置新值
        compression_and_encryption_ &= ~encryption_method_mask; // 清除低4位
        compression_and_encryption_ |= encryption_method_bits & encryption_method_mask; // 设置新的加密方法位
    }

    std::array<uint8_t, 16> local_file_header::get_salt() const
    {
        std::array<uint8_t, 16> salt{};
        for (size_t i = 0; i < 16; i++)
        {
            salt[i] = salt_[i];
        }
        return salt;
    }

    void local_file_header::set_salt(const std::array<uint8_t, 16>& salt)
    {
        for (size_t i = 0; i < 16; i++)
        {
            salt_[i] = salt[i];
        }
    }

    uint16_t local_file_header::get_link_name_length() const
    {
        return make_word({link_name_length_[0], link_name_length_[1]});
    }

    void local_file_header::set_link_name_length(uint16_t link_name_length)
    {
        const two_byte bytes = to_bytes(link_name_length);
        link_name_length_[0] = std::get<0>(bytes);
        link_name_length_[1] = std::get<1>(bytes);
    }

    uint64_t local_file_header::get_original_file_size() const
    {
        return make_qword({original_file_size_[0], original_file_size_[1],
                      original_file_size_[2], original_file_size_[3],
                      original_file_size_[4], original_file_size_[5],
                      original_file_size_[6], original_file_size_[7]});
    }

    void local_file_header::set_original_file_size(uint64_t original_file_size)
    {
        const eight_byte bytes = to_bytes(original_file_size);

        original_file_size_[0] = std::get<0>(bytes);
        original_file_size_[1] = std::get<1>(bytes);
        original_file_size_[2] = std::get<2>(bytes);
        original_file_size_[3] = std::get<3>(bytes);
        original_file_size_[4] = std::get<4>(bytes);
        original_file_size_[5] = std::get<5>(bytes);
        original_file_size_[6] = std::get<6>(bytes);
        original_file_size_[7] = std::get<7>(bytes);
    }

    uint64_t local_file_header::get_file_size() const
    {
        return make_qword({file_size_[0], file_size_[1],
                      file_size_[2], file_size_[3],
                      file_size_[4], file_size_[5],
                      file_size_[6], file_size_[7]});
    }

    void local_file_header::set_file_size(uint64_t file_size)
    {
        const eight_byte bytes = to_bytes(file_size);

        file_size_[0] = std::get<0>(bytes);
        file_size_[1] = std::get<1>(bytes);
        file_size_[2] = std::get<2>(bytes);
        file_size_[3] = std::get<3>(bytes);
        file_size_[4] = std::get<4>(bytes);
        file_size_[5] = std::get<5>(bytes);
        file_size_[6] = std::get<6>(bytes);
        file_size_[7] = std::get<7>(bytes);
    }

    uint16_t local_file_header::get_file_name_length() const
    {
        return make_word({file_name_length_[0], file_name_length_[1]});
    }

    void local_file_header::set_file_name_length(uint16_t file_name_length)
    {
        const two_byte bytes = to_bytes(file_name_length);
        file_name_length_[0] = std::get<0>(bytes);
        file_name_length_[1] = std::get<1>(bytes);
    }

    std::string local_file_header::get_link_name() const
    {
        return std::string(link_name_.get(),link_name_.get()+get_link_name_length());
    }

    void local_file_header::set_link_name(const std::string& link_name)
    {
        link_name_ = std::make_unique<byte[]>(get_link_name_length());
        for (word i = 0; i < get_link_name_length(); i++)
        {
            link_name_[i] = link_name[i];
        }
    }

    std::string local_file_header::get_file_name() const
    {
        return std::string(file_name_.get(),file_name_.get()+get_file_name_length());
    }

    void local_file_header::set_file_name(const std::string& file_name)
    {
        file_name_ = std::make_unique<byte[]>(get_file_name_length());
        for (word i = 0; i < get_file_name_length(); i++)
        {
            file_name_[i] = file_name[i];
        }
    }

    bool local_file_header::check() const
    {
        auto tmp_checksum = get_checksum();

        const std::vector<std::pair<const byte*, size_t>> datas{
            {uid_, sizeof(uid_)},
            {gid_, sizeof(gid_)},
            {uname_, sizeof(uname_)},
            {gname_, sizeof(gname_)},
            {creation_time_, sizeof(creation_time_)},
            {last_modification_time_, sizeof(last_modification_time_)},
            {last_access_time_, sizeof(last_access_time_)},
            {file_type_and_permissions_, sizeof(file_type_and_permissions_)},
            {crc_32_, sizeof(crc_32_)},
            {&compression_and_encryption_, sizeof(compression_and_encryption_)},
            {salt_, sizeof(salt_)},
            {original_file_size_, sizeof(original_file_size_)},
            {file_size_, sizeof(file_size_)},
            {link_name_length_, sizeof(link_name_length_)},
            {file_name_length_, sizeof(file_name_length_)},
            {link_name_.get(),get_link_name_length()},
            {file_name_.get(),get_file_name_length()}
        };

        return tmp_checksum == (calculate_checksum(datas));
    }
} // data_packet