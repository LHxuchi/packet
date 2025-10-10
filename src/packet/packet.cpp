//
// Created by hyh on 2025/10/8.
//

#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <climits>

#include "../../include/packet/packet.h"

#include <fstream>

#include "../../include/utils/byte_conversion.h"
#include "../../include/utils/crc_32.h"
#include "../../include/file_system/get_entries.h"

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

data_packet::packet data_packet::make_packet(const std::filesystem::path& path)
{
    namespace fs = std::filesystem;

    packet pkt;

    auto entries = get_entries(path);

    pkt.packets().reserve(entries.size());

    for (const auto& entry : entries)
    {
        local_packet tmp;
        struct stat file_stat{};
        if (lstat(path.c_str(), &file_stat) != 0)
        {
            throw std::runtime_error("cannot stat " + path.string());
        }

        tmp.set_uid(file_stat.st_uid);
        tmp.set_gid(file_stat.st_gid);
        tmp.set_uname(getpwuid(file_stat.st_uid)->pw_name);
        tmp.set_gname(getgrgid(file_stat.st_gid)->gr_name);
        tmp.refresh_creation_time();
        tmp.set_last_modification_time(file_stat.st_mtime);
        tmp.set_last_access_time(file_stat.st_atime);
        tmp.set_permissions(entry.symlink_status().permissions());
        tmp.set_file_type(entry.symlink_status().type());
        tmp.set_compression_method(local_file_header::compression_method::None);
        tmp.set_encryption_method(local_file_header::encryption_method::None);
        tmp.set_salt(std::array<uint8_t, 16>{});
        tmp.set_original_file_size(file_stat.st_size);
        tmp.set_file_size(file_stat.st_size);

        std::string file_name = entry.path().lexically_relative(path);

        tmp.set_file_name_length(file_name.size());
        tmp.set_file_name(file_name);

        switch (entry.symlink_status().type())
        {
        case fs::file_type::regular:
        case fs::file_type::block:
        case fs::file_type::character:
            {
                std::fstream file(entry.path(), std::ios::in | std::ios::binary);
                auto buffer = std::make_unique<byte[]>(entry.file_size());
                file.read(reinterpret_cast<char*>(buffer.get()), static_cast<long>(entry.file_size()));
                tmp.set_data(std::move(buffer));
                tmp.set_link_name_length(0);
                tmp.set_link_name(std::string{});
                break;
            }
        case fs::file_type::fifo:
        case fs::file_type::directory:
            {
                tmp.set_file_size(0);
                tmp.set_original_file_size(0);
                tmp.set_link_name_length(0);
                tmp.set_link_name(std::string{});
                break;
            }
        case fs::file_type::symlink:
            {
                char buf[PATH_MAX];  // PATH_MAX定义了系统最大路径长度
                ssize_t len;
                // 读取软链接目标路径
                len = readlink(entry.path().c_str(), buf, sizeof(buf) - 1);
                if (len == -1) {
                    throw std::runtime_error("cannot readlink " + path.string());
                }
                buf[len] = '\0';  // 手动添加字符串结束符
                tmp.set_link_name_length(len);
                tmp.set_link_name(std::string(buf));
                tmp.set_file_size(0);
                tmp.set_original_file_size(0);
                break;
            }
        case fs::file_type::none:
            throw std::runtime_error(entry.path().string() + " file type not supported");
        default:
            continue;
        }

        tmp.refresh_crc_32();
        tmp.refresh_checksum();

        pkt.packets().emplace_back(std::move(tmp));
    }

    pkt.set_version(1);
    pkt.refresh_creation_time();
    pkt.refresh_file_number();
    pkt.refresh_file_size();
    pkt.refresh_original_file_size();
    pkt.refresh_crc_32();
    pkt.refresh_checksum();

    return pkt;
}
