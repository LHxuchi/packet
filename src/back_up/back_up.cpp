//
// Created by hyh on 2025/12/26.
//

#include "../../include/back_up/back_up.h"
#include "../../include/packet/packet.h"
#include "../../include/file_system/get_entries.h"
#include <fstream>
#include "../../include/compression_method/huffman.h"
#include "../../include/compression_method/lz77.h"
#include "../../include/encryption_method/encryption.h"

// 匿名命名空间：限制以下函数仅在当前编译单元（.cpp文件）内可见，避免命名冲突
namespace
{
    /**
     * @brief 字符串类型的压缩方法转换为枚举类型的压缩方法
     * @param method 字符串格式的压缩方法（支持 "LZ77"、"HUFFMAN"、"NONE"）
     * @return 对应的 data_packet::local_file_header::compression_method 枚举值
     * @throw std::invalid_argument 当传入不识别的压缩方法字符串时抛出异常
     */
    data_packet::local_file_header::compression_method c_method(const std::string& method)
    {
        if (method == "LZ77")
        {
            return data_packet::local_file_header::compression_method::LZ77;
        }
        else if (method == "HUFFMAN")
        {
            return data_packet::local_file_header::compression_method::HUFFMAN;
        }
        else if (method == "NONE")
        {
            return data_packet::local_file_header::compression_method::None;
        }
        else
        {
            throw std::invalid_argument("compression method was not recognised.");
        }
    }

    /**
     * @brief 字符串类型的加密方法转换为枚举类型的加密方法
     * @param method 字符串格式的加密方法（支持 "AES_256_CBC"、"NONE"）
     * @return 对应的 data_packet::local_file_header::encryption_method 枚举值
     * @throw std::invalid_argument 当传入不识别的加密方法字符串时抛出异常
     */
    data_packet::local_file_header::encryption_method e_method(const std::string& method)
    {
        if (method == "AES_256_CBC")
        {
            return data_packet::local_file_header::encryption_method::AES_256_CBC;
        }
        else if (method == "NONE")
        {
            return data_packet::local_file_header::encryption_method::None;
        }
        else
        {
            throw std::invalid_argument("encryption method was not recognised.");
        }
    }

    /**
     * @brief 枚举类型的压缩方法转换为字符串类型
     * @param method 枚举格式的压缩方法
     * @return 对应的字符串（"LZ77"、"HUFFMAN"、"NONE" 或 "UNKNOWN"）
     */
    std::string to_string(data_packet::local_file_header::compression_method method)
    {
        switch (method)
        {
            case data_packet::local_file_header::compression_method::LZ77:
                return "LZ77";
            case data_packet::local_file_header::compression_method::HUFFMAN:
                return "HUFFMAN";
            case data_packet::local_file_header::compression_method::None:
                return "NONE";
        }

        return "UNKNOWN";
    }

    /**
     * @brief 枚举类型的加密方法转换为字符串类型
     * @param method 枚举格式的加密方法
     * @return 对应的字符串（"AES 256 CBC"、"NONE" 或 "UNKNOWN"）
     */
    std::string to_string(data_packet::local_file_header::encryption_method method)
    {
        switch (method)
        {
            case data_packet::local_file_header::encryption_method::AES_256_CBC:
                return "AES 256 CBC";
            case data_packet::local_file_header::encryption_method::None:
                return "NONE";
            case data_packet::local_file_header::encryption_method::my_method:
                return "UNKNOWN";
        }

        return "UNKNOWN";
    }
}

/**
 * @brief 执行文件备份核心功能：打包源目录文件，进行压缩、加密后写入目标文件
 * @param source 源目录路径（待备份的目录，必须存在）
 * @param destination 目标文件路径（备份文件输出路径）
 * @param compression_method 压缩方法字符串（"LZ77"、"HUFFMAN"、"NONE"）
 * @param encryption_method 加密方法字符串（"AES_256_CBC"、"NONE"）
 * @param password 加密/解密密码（当加密方法为 AES_256_CBC 时有效）
 * @param not_including_files 需排除的文件列表（按换行符 \n 分隔多个文件名）
 * @return 执行结果：成功返回 "OK"，失败返回异常信息字符串
 */
std::string data_packet::back_up(
    const std::filesystem::path& source,
    const std::filesystem::path& destination,
    const std::string& compression_method,
    const std::string& encryption_method,
    const std::string& password,
    const std::string& not_including_files)
{
    try
    {
        // 第一步：校验源目录是否存在，不存在则抛出异常
        if (!std::filesystem::exists(source))
        {
            throw std::invalid_argument("source directory does not exist.");
        }

        // 第二步：打包源目录所有文件为 packet 结构体
        auto pkt = make_packet(source);

        // 第三步：解析排除文件列表，存储到有序集合中（方便快速查找）
        std::set<std::string> filtered_files; // 排除文件集合（自动去重、有序）
        auto begin = not_including_files.begin(), end = not_including_files.begin();

        // 按换行符 \n 分割字符串，提取每个需排除的文件名
        while (end != not_including_files.end())
        {
            if (*end == '\n')
            {
                filtered_files.insert({begin, end}); // 插入一段[begin, end)的文件名
                begin = end + 1; // 移动起始迭代器到下一个文件名开头
            }
            ++end;
        }
        // 处理最后一个文件名（若字符串末尾无换行符）
        if (begin != end)
        {
            filtered_files.insert({begin, end});
        }

        // 第四步：遍历 packet 中的本地文件包，移除需排除的文件
        for (auto it = pkt.packets().begin(); it != pkt.packets().end();)
        {
            // 若当前文件在排除列表中，删除该元素并更新迭代器
            if (filtered_files.find(it->info().get_file_name()) != filtered_files.end())
            {
                it = pkt.packets().erase(it); // erase 返回下一个有效迭代器，无需手动++
                continue;
            }
            ++it; // 非排除文件，迭代器正常后移
        }

        // 第五步：更新 packet 中的文件数量（删除排除文件后，文件总数发生变化）
        pkt.refresh_file_number();

        // 第六步：遍历所有本地文件包，依次执行 压缩 -> 加密 操作
        for (auto& local_pkt:pkt.packets())
        {
            // 1. 设置当前文件包的压缩方法和加密方法（字符串转枚举）
            local_pkt.set_compression_method(c_method(compression_method));
            local_pkt.set_encryption_method(e_method(encryption_method));

            // 2. 获取当前文件包的原始数据和文件大小
            auto data = local_pkt.get_data().get();
            auto size = local_pkt.info().get_file_size();

            // 存储压缩/加密后的数据流和大小（初始化为原始数据大小，空指针）
            std::pair<std::unique_ptr<byte[]>, size_t> stream = {nullptr, size};

            // 3. 根据压缩方法执行对应压缩操作
            switch (local_pkt.info().get_compression_method())
            {
                case local_file_header::compression_method::LZ77:
                    // LZ77 压缩：输入原始数据指针范围，返回压缩后的数据流和大小
                    stream = lz77_compress(data, data + size);
                    break;
                case local_file_header::compression_method::HUFFMAN:
                    // Huffman 压缩：输入原始数据指针范围，返回压缩后的数据流和大小
                    stream = Huffman_compress(data, data + size);
                    break;
                case local_file_header::compression_method::None:
                    // 不压缩：保持原始数据不变，无需处理
                    break;
            }

            // 4. 若压缩成功（数据流非空），更新文件包的数据流和文件大小
            if (stream.first != nullptr)
            {
                local_pkt.set_data(std::move(stream.first)); // 移动语义，避免拷贝
                local_pkt.set_file_size(stream.second);      // 更新为压缩后的文件大小
            }

            // 5. 重新获取压缩后的数据和大小，用于后续加密操作
            data = local_pkt.get_data().get();
            size = local_pkt.info().get_file_size();
            stream = {nullptr, size}; // 重置数据流，准备存储加密后数据

            // 6. 根据加密方法执行对应加密操作
            switch (local_pkt.info().get_encryption_method())
            {
                case local_file_header::encryption_method::AES_256_CBC:
                    // AES_256_CBC 加密：输入压缩后数据、密码，返回加密后的数据流和大小
                    stream = encrypt(data, data + size, password);
                    break;
                case local_file_header::encryption_method::None:
                case local_file_header::encryption_method::my_method:
                    // 不加密/自定义方法：保持数据不变，无需处理
                    break;
            }

            // 7. 若加密成功（数据流非空），更新文件包的数据流和文件大小
            if (stream.first != nullptr)
            {
                local_pkt.set_data(std::move(stream.first)); // 移动语义，避免拷贝
                local_pkt.set_file_size(stream.second);      // 更新为加密后的文件大小
            }

            // 8. 刷新当前文件包的校验信息和时间信息，保证数据一致性
            local_pkt.refresh_crc_32();          // 刷新 CRC32 校验值
            local_pkt.refresh_creation_time();   // 刷新文件创建时间
            local_pkt.refresh_checksum();        // 刷新校验和
        }

        // 第七步：刷新整个 packet 的全局信息，保证备份文件的完整性
        pkt.refresh_original_file_size(); // 刷新原始总文件大小（未压缩/加密前）
        pkt.refresh_file_size();          // 刷新备份后总文件大小（压缩/加密后）
        pkt.refresh_creation_time();      // 刷新备份包的创建时间
        pkt.refresh_crc_32();             // 刷新备份包的 CRC32 校验值
        pkt.refresh_checksum();           // 刷新备份包的校验和

        // 第八步：以二进制模式打开目标文件，写入备份包数据
        std::ofstream out(destination, std::ios::binary);
        if (!out.is_open())
        {
            throw std::runtime_error("Could not open output file " + destination.string());
        }
        out << pkt; // 重载 << 运算符，将 packet 序列化写入文件

        return {"OK"}; // 备份成功，返回 OK
    }
    catch(const std::exception& e)
    {
        // 捕获所有异常，返回异常信息字符串
        return e.what();
    }
}

/**
 * @brief 获取备份文件的详细信息（版本、大小、压缩/加密方法、包含文件名等）
 * @param path 备份文件路径（需存在且为有效备份包）
 * @return 备份文件信息字符串（格式化输出），失败时返回异常信息字符串
 */
std::string data_packet::info(const std::filesystem::path& path)
{
    try
    {
        // 第一步：以二进制模式打开备份文件
        std::ifstream input(path, std::ios::binary);
        if (!input.is_open())
        {
            throw std::runtime_error("Could not open output file " + path.string());
        }

        // 第二步：反序列化备份文件数据到 packet 结构体
        packet pkt;
        input >> pkt; // 重载 >> 运算符，从文件读取并解析 packet

        // 第三步：格式化拼接备份文件信息
        std::string result;

        // 拼接全局信息：版本、文件大小、原始大小、创建时间、文件数量
        result.append(std::format("version:{}\n",pkt.info().get_version()));
        result.append(std::format("file size:{}\n",pkt.info().get_file_size()));
        result.append(std::format("original file size:{}\n",pkt.info().get_original_file_size()));
        result.append(std::format("creation time:{}\n",pkt.info().get_creation_time()));
        result.append(std::format("file number:{}\n",pkt.info().get_file_number()));
        // 拼接压缩方法和加密方法（取第一个本地文件包的方法，默认所有文件方法一致）
        result.append(std::format("compression method:{}\n",to_string(pkt.packets().front().info().get_compression_method())));
        result.append(std::format("encryption method:{}\n",to_string(pkt.packets().front().info().get_encryption_method())));
        // 拼接所有包含的文件名列表
        result.append("all file names:\n");
        for (const auto& local_pkt:pkt.packets())
        {
            result.append(local_pkt.info().get_file_name() + "\n");
        }

        return result; // 返回格式化后的信息字符串
    }
    catch (const std::exception& e)
    {
        // 捕获异常，返回异常信息
        return e.what();
    }

}

/**
 * @brief 执行备份文件恢复核心功能：读取备份文件，解密、解压后还原到目标目录
 * @param source 备份文件路径（待恢复的备份文件，必须存在且有效）
 * @param destination 目标目录路径（恢复后的文件输出目录）
 * @param password 解密密码（与备份时的密码一致，AES_256_CBC 加密时有效）
 * @return 执行结果：成功返回 "OK"，失败返回异常信息字符串
 */
std::string data_packet::restore_backup(const std::filesystem::path& source, const std::filesystem::path& destination,
                                        const std::string& password)
{
    try
    {
        // 第一步：以二进制模式打开备份文件
        std::ifstream input(source, std::ios::binary);
        if (!input.is_open())
        {
            throw std::runtime_error("Could not open output file " + source.string());
        }

        // 第二步：反序列化备份文件到 packet 结构体
        packet pkt;
        input >> pkt;

        // 第三步：遍历所有本地文件包，依次执行 解密 -> 解压 操作
        for (auto& local_pkt:pkt.packets())
        {
            // 1. 获取加密后的数据流和大小
            auto data = local_pkt.get_data().get();
            auto size = local_pkt.info().get_file_size();

            // 存储解密/解压后的数据流和大小
            std::pair<std::unique_ptr<byte[]>, size_t> stream = {nullptr,size};

            // 2. 根据加密方法执行对应解密操作（先解密，后解压）
            switch (local_pkt.info().get_encryption_method())
            {
            case local_file_header::encryption_method::AES_256_CBC:
                {
                    // AES_256_CBC 解密：输入加密数据、密码，返回解密后的数据流
                    stream = decrypt(data,data+size,password);
                    // 解密失败（密码错误等），抛出异常
                    if (stream.first == nullptr)
                    {
                        throw std::runtime_error("Fail to decrypt the file " + destination.string() + ". Wrong password");
                    }
                    break;
                }
            default:
                // 不加密/其他方法：保持数据不变，无需处理
                break;
            }

            // 3. 若解密成功，更新文件包的数据流和大小
            if (stream.first != nullptr)
            {
                local_pkt.set_data(std::move(stream.first));
                local_pkt.set_file_size(stream.second);
            }

            // 4. 重新获取解密后的数据和大小，用于后续解压操作
            data = local_pkt.get_data().get();
            size = local_pkt.info().get_file_size();
            stream = {nullptr,size}; // 重置数据流

            // 5. 根据压缩方法执行对应解压操作
            switch (local_pkt.info().get_compression_method())
            {
            case local_file_header::compression_method::LZ77:
                // LZ77 解压：输入压缩数据，返回原始数据流
                stream = lz77_decompress(data,data+size);
                break;
            case local_file_header::compression_method::HUFFMAN:
                // Huffman 解压：输入压缩数据，返回原始数据流
                stream = Huffman_decompress(data,data+size);
                break;
            case local_file_header::compression_method::None:
                // 不压缩：保持数据不变，无需处理
                break;
            }

            // 6. 若解压成功，更新文件包的数据流和大小（还原为原始数据）
            if (stream.first != nullptr)
            {
                local_pkt.set_data(std::move(stream.first));
                local_pkt.set_file_size(stream.second);
            }

        }

        // 第四步：将还原后的 packet 解压到目标目录
        unpack_packet(destination,pkt);

        return "OK"; // 恢复成功，返回 OK
    }
    catch (const std::exception& e)
    {
        // 捕获异常，返回异常信息
        return e.what();
    }
}