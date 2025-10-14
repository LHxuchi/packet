//
// Created by hyh on 2025/10/8.
//

#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <fstream>
#include <map>
#include <sys/sysmacros.h>

#include "../../include/packet/packet.h"

#include <cstring>
#include <format>

#include "../../include/utils/byte_conversion.h"
#include "../../include/utils/crc_32.h"
#include "../../include/file_system/get_entries.h"

namespace
{
    /**
     * @brief 抽取出来的一个填表过程，不值得复用，丢匿名空间就行
     * @param packet 需要被填写的包文件
     * @param file_stat 文件描述符
     * @param path 文件当前绝对路径
     * @param root_path 根路径
     */
    void fill_in_local_header(data_packet::local_packet& packet,const struct stat& file_stat,
                              const std::filesystem::path& path , const std::filesystem::path& root_path)
    {
        namespace fs = std::filesystem;
        // 设置用户和组信息
        packet.set_uid(file_stat.st_uid);
        packet.set_gid(file_stat.st_gid);
        packet.set_uname(getpwuid(file_stat.st_uid)->pw_name); // 获取用户名
        packet.set_gname(getgrgid(file_stat.st_gid)->gr_name); // 获取组名

        // 设置时间信息
        packet.refresh_creation_time();
        packet.set_last_modification_time(file_stat.st_mtime); // 最后修改时间
        packet.set_last_access_time(file_stat.st_atime);       // 最后访问时间

        // 设置权限和文件类型
        packet.set_permissions(fs::directory_entry(path).symlink_status().permissions());
        packet.set_file_type(fs::directory_entry(path).symlink_status().type()); // 不跟随软链接获取文件类型

        // 设置压缩和加密方法（当前均为None）
        packet.set_compression_method(data_packet::local_file_header::compression_method::None);
        packet.set_encryption_method(data_packet::local_file_header::encryption_method::None);
        packet.set_salt(std::array<uint8_t, 16>{}); // 设置空的盐值

        // 设置文件大小信息
        packet.set_original_file_size(file_stat.st_size);
        packet.set_file_size(file_stat.st_size);

        // 计算相对于根路径的文件名（不解析软链接）
        const std::string file_name = path.lexically_relative(root_path);

        packet.set_file_name_length(file_name.size());
        packet.set_file_name(file_name);
    }

    /**
     * @brief 处理不同文件的存储方式，不值得复用，丢匿名空间就行
     * @param packet 包文件
     * @param entry 指定访问目录
     * @param root_path 根路径
     * @param file_stat 文件描述符
     */
    void fill_in_local_header_link(data_packet::local_packet& packet,
                                   const std::filesystem::directory_entry& entry,
                                   const std::filesystem::path& root_path, const struct stat& file_stat)
    {
        namespace fs = std::filesystem;
        using namespace data_packet;
        switch (entry.symlink_status().type())
        {
        case fs::file_type::block:     // 块设备文件
        case fs::file_type::character: // 字符设备文件
            {
                /* 在文件内容中存储主设备号与次设备号 */
                auto buffer = std::make_unique<byte[]>(8); // 主设备号4字节次设备号4字节
                uint32_t main_dev = major(file_stat.st_dev);
                uint32_t sub_dev = minor(file_stat.st_dev);
                four_byte bytes = to_bytes(main_dev);
                buffer[0] = std::get<0>(bytes);
                buffer[1] = std::get<1>(bytes);
                buffer[2] = std::get<2>(bytes);
                buffer[3] = std::get<3>(bytes);

                bytes = to_bytes(sub_dev);
                buffer[4] = std::get<0>(bytes);
                buffer[5] = std::get<1>(bytes);
                buffer[6] = std::get<2>(bytes);
                buffer[7] = std::get<3>(bytes);

                packet.set_data(std::move(buffer)); // 设置文件数据
                // 文件长度设置为8字节
                packet.set_original_file_size(8);
                packet.set_file_size(8);

                // 不存在链接名
                packet.set_link_name_length(0);
                packet.set_link_name(std::string{});
                break;
            }
        case fs::file_type::regular:   // 常规文件
            {
                // 读取文件内容到缓冲区
                std::fstream file(entry.path(), std::ios::in | std::ios::binary);
                auto buffer = std::make_unique<byte[]>(entry.file_size());
                file.read(reinterpret_cast<char*>(buffer.get()), static_cast<long>(entry.file_size()));
                packet.set_data(std::move(buffer)); // 设置文件数据
                packet.set_link_name_length(0);
                packet.set_link_name(std::string{});
                break;
            }
        case fs::file_type::fifo:     // 命名管道
        case fs::file_type::directory: // 目录
            {
                // 目录和管道没有文件内容
                packet.set_file_size(0);
                packet.set_original_file_size(0);
                packet.set_link_name_length(0);
                packet.set_link_name(std::string{});
                break;
            }
        case fs::file_type::symlink:  // 软链接
            {
                auto link_name = fs::relative(entry.path(), root_path);
                packet.set_link_name_length(link_name.string().size());
                packet.set_link_name(link_name.string());
                packet.set_file_size(0);
                packet.set_original_file_size(0);
                break;
            }
        case fs::file_type::none:     // 未知或不受支持的文件类型
            throw std::runtime_error(entry.path().string() + " file type not supported");
        default:
            return;
            // 跳过其他未处理的文件类型
        }
    }
}

/**
 * @brief 刷新数据包的总文件大小（包含头部）
 *
 * 计算数据包头大小加上所有本地数据包的文件大小和头部大小的总和
 */
void data_packet::packet::refresh_file_size()
{
    qword size = _header.header_size(); // 起始大小为包头大小
    for (const auto& local_packet : _packets)
    {
        size += local_packet.info().get_file_size(); // 累加每个数据包的文件大小
        size += local_packet.info().header_size();   // 累加每个数据包的头部大小
    }
    _header.set_file_size(size); // 设置总文件大小
}

/**
 * @brief 刷新数据包的原始文件大小（包含头部）
 *
 * 计算数据包头大小加上所有本地数据包的原始文件大小和头部大小的总和
 */
void data_packet::packet::refresh_original_file_size()
{
    qword size = _header.header_size(); // 起始大小为包头大小
    for (const auto& local_packet : _packets)
    {
        size += local_packet.info().get_original_file_size(); // 累加每个数据包的原始文件大小
        size += local_packet.info().header_size();           // 累加每个数据包的头部大小
    }
    _header.set_original_file_size(size); // 设置总原始文件大小
}

/**
 * @brief 刷新数据包的CRC32校验值
 *
 * 基于所有本地数据包的CRC32值计算整个数据包的CRC32校验值
 */
void data_packet::packet::refresh_crc_32()
{
    // 创建缓冲区存储所有数据包的CRC32值（每个CRC32为4字节）
    auto buffer = std::make_unique<byte[]>(_packets.size() * 4);
    size_t offset = 0;

    // 将每个数据包的CRC32值转换为字节并存入缓冲区
    for (const auto& local_packet : _packets)
    {
        auto bytes = to_bytes(local_packet.info().get_crc_32());
        buffer[offset++] = std::get<0>(bytes);
        buffer[offset++] = std::get<1>(bytes);
        buffer[offset++] = std::get<2>(bytes);
        buffer[offset++] = std::get<3>(bytes);
    }

    // 计算整个缓冲区的CRC32值并设置为数据包的CRC32
    _header.set_crc_32(CRC_calculate(reinterpret_cast<uint8_t*>(buffer.get()), offset));
}

/**
 * @brief 根据指定路径创建数据包
 * @param path 要打包的文件或目录路径
 * @param get_entries
 * @return 包含路径下所有文件信息的数据包
 * @throws std::runtime_error 当无法获取文件状态或读取软链接时抛出异常
 *
 * 该函数遍历指定路径下的所有条目（文件、目录、软链接等），
 * 为每个条目创建本地数据包，收集文件属性、权限、内容等信息
 */
data_packet::packet data_packet::make_packet(const std::filesystem::path& path, const get_entries_t& get_entries)
{
    namespace fs = std::filesystem;

    packet pkt; // 创建主数据包

    // 获取路径下的所有条目（不跟随软链接）
    auto entries = get_entries(path,[](const std::filesystem::directory_entry&){return false;});

    // 预留空间以避免多次重新分配
    pkt.packets().reserve(entries.size());

    // 处理硬链接的映射表
    std::map<decltype(stat::st_ino),fs::path> files;

    // 遍历每个条目并创建对应的本地数据包
    for (const auto& entry : entries)
    {
        local_packet tmp;
        struct stat file_stat{};

        // 获取文件状态信息（不跟随软链接）
        if (lstat(entry.path().c_str(), &file_stat) != 0)
        {
            throw std::runtime_error("cannot stat " + path.string());
        }

        // 填写包文件头，不包括软链接
        fill_in_local_header(tmp, file_stat, entry.path(), path);

        if (files.find(file_stat.st_ino) != files.end())
        {
            // 处理硬链接
            auto buffer = std::make_unique<byte[]>(11);
            memcpy(buffer.get(), "\nhard_link\n", 11);
            tmp.set_file_size(11);
            tmp.set_original_file_size(11);
            tmp.set_data(std::move(buffer));
            tmp.set_link_name_length(files.at(file_stat.st_ino).string().size());
            tmp.set_link_name(files.at(file_stat.st_ino).string());
        }
        else
        {
            // 记录inode以及其对应路径
            files.insert({file_stat.st_ino,tmp.info().get_file_name()});
            // 根据文件类型处理不同类型的数据（非硬链接）
            fill_in_local_header_link(tmp, entry, path, file_stat);
        }

        // 刷新当前本地数据包的CRC32和校验和
        tmp.refresh_crc_32();
        tmp.refresh_checksum();

        // 将本地数据包添加到主数据包中
        pkt.packets().emplace_back(std::move(tmp));
    }

    // 设置主数据包的元数据信息
    pkt.set_version(1); // 设置数据包版本
    pkt.refresh_creation_time();          // 刷新创建时间
    pkt.refresh_file_number();            // 刷新文件数量
    pkt.refresh_file_size();              // 刷新总文件大小
    pkt.refresh_original_file_size();     // 刷新总原始文件大小
    pkt.refresh_crc_32();                 // 刷新CRC32校验值
    pkt.refresh_checksum();               // 刷新校验和

    return pkt;
}


/**
 * @brief 流式输出
 * @param os 指定输出流
 * @param packet *this
 * @return 指定输出流
 */
std::ostream& data_packet::operator<<(std::ostream& os, const packet& packet)
{
    os.write(packet.info().get_buffer().get(),static_cast<long>(packet.info().header_size()));

    for (const auto& local_pkt : packet.packets())
    {
        os.write(local_pkt.info().get_buffer().get(),static_cast<long>(local_pkt.info().header_size()));

        os.write(local_pkt.get_data().get(),static_cast<long>(local_pkt.info().get_file_size()));
    }

    return os;
}

/**
 * @brief 流式输入
 * @param is 指定输入流
 * @param packet *this
 * @return 指定输入流
 */
std::istream& data_packet::operator>>(std::istream& is, packet& packet)
{
    std::unique_ptr<byte[]> buffer{nullptr};
    buffer = std::make_unique<byte[]>(file_header::SIZE);

    // 读取总文件头前检查
    if (!is.good()) {
        throw std::runtime_error("Stream is in error state before reading header");
    }

    // 读取总文件头
    is.read(buffer.get(), file_header::SIZE);
    packet.set_header_buffer(buffer.get());

    if (!packet.info().check())
    {
        throw std::runtime_error("packet header is not valid");
    }

    // 分配包文件个数
    packet.packets().resize(packet.info().get_file_number());

    // 填充包文件
    for (auto& local_pkt : packet.packets())
    {
        // 读取本地文件头定长字段信息
        buffer = std::make_unique<byte[]>(local_file_header::SIZE);
        is.read(buffer.get(), local_file_header::SIZE);
        local_pkt.set_header_buffer(buffer.get());

        // 读取变长字段link_name 与 file_name
        buffer = std::make_unique<byte[]>(local_pkt.info().get_link_name_length());
        is.read(buffer.get(), local_pkt.info().get_link_name_length());
        local_pkt.set_link_name(buffer.get());

        // 填写文件内容
        buffer = std::make_unique<byte[]>(local_pkt.info().get_file_name_length());
        is.read(buffer.get(), local_pkt.info().get_file_name_length());
        local_pkt.set_file_name(buffer.get());

        // 校验
        if (!local_pkt.info().check())
        {
            throw std::runtime_error("local packet file header is not valid");
        }

        // 读取文件信息
        buffer = std::make_unique<byte[]>(local_pkt.info().get_file_size());
        is.read(buffer.get(), static_cast<long>(local_pkt.info().get_file_size()));
        local_pkt.set_data(std::move(buffer));
    }

    return is;
}

void data_packet::unpack_packet(const std::filesystem::path& path, const packet& pkt)
{
    namespace fs = std::filesystem;

    // 优先还原目录结构
    for (const auto& local_pkt : pkt.packets())
    {
        if (local_pkt.info().get_file_type() == fs::file_type::directory)
        {
            fs::create_directory(path / local_pkt.info().get_file_name());
            fs::permissions(path / local_pkt.info().get_file_name(),local_pkt.info().get_permissions());
        }
    }

    // 还原正常文件
    for (const auto& local_pkt : pkt.packets())
    {
        if (local_pkt.info().get_link_name_length() > 0 || local_pkt.info().get_file_type() == fs::file_type::directory)
        {
            continue;
        }
        switch (local_pkt.info().get_file_type())
        {
        case fs::file_type::regular:
            {
                // 创建文件并写入数据
                std::ofstream file(path / local_pkt.info().get_file_name(), std::ios::binary);
                file.write(local_pkt.get_data().get(), static_cast<long>(local_pkt.info().get_file_size()));
                file.close();

                // 设置文件权限
                fs::permissions(path / local_pkt.info().get_file_name(),local_pkt.info().get_permissions());
                break;
            }
        case fs::file_type::character:
        case fs::file_type::block:
            {
                // 读取主设备号与次设备号
                auto buffer = local_pkt.get_data().get();
                uint32_t main_dev = make_dword({buffer[0],buffer[1],buffer[2],buffer[3]});
                uint32_t sub_dev = make_dword({buffer[4],buffer[5],buffer[6],buffer[7]});

                // 读取文件名
                auto file_name = (path / local_pkt.info().get_file_name()).string();

                // 确认文件类型
                int type = local_pkt.info().get_file_type() == fs::file_type::character? S_IFCHR : S_IFBLK;

                // 创建设备文件
                mknod(file_name.c_str(), type, makedev(main_dev, sub_dev));

                // 设置权限
                fs::permissions(file_name,local_pkt.info().get_permissions());

                break;
            }
        case fs::file_type::fifo:
            {
                auto file_name = (path / local_pkt.info().get_file_name()).string();
                mkfifo(file_name.c_str(), 0000);
                fs::permissions(file_name,local_pkt.info().get_permissions());
                break;
            }
        default:
            throw std::runtime_error("File type not recognized " + local_pkt.info().get_file_name());
        }
    }

    // 还原硬链接关系
    for (const auto& local_pkt : pkt.packets())
    {
        if (local_pkt.info().get_link_name_length() <= 0)
        {
            continue;
        }
        if (local_pkt.info().get_file_size() > 0)
        {
            // 还原硬链接
            auto target_name = path / local_pkt.info().get_link_name();
            auto link_name = path / local_pkt.info().get_file_name();
            fs::create_hard_link(target_name,link_name);
            fs::permissions(link_name,local_pkt.info().get_permissions());
        }
    }

    // 还原软链接
    for (const auto& local_pkt : pkt.packets())
    {
        if (local_pkt.info().get_link_name_length() <= 0)
        {
            continue;
        }
        if (local_pkt.info().get_file_size() == 0)
        {
            // 还原软链接
            auto target_name = path / local_pkt.info().get_link_name();
            auto link_name = path / local_pkt.info().get_file_name();
            fs::create_symlink(target_name,link_name);
            fs::permissions(link_name,local_pkt.info().get_permissions());
        }
    }
}
