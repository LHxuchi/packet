//
// Created by hyh on 2025/12/26.
//

#ifndef DATA_BACK_UP_BACK_UP_H
#define DATA_BACK_UP_BACK_UP_H

#include <filesystem>

namespace data_packet
{
    /**
     * @brief 将指定路径下的文件进行打包备份，并将备份包写入指定目录
     * @param source 需要打包的指定目录
     * @param destination 打包后输出的指定目录
     * @param compression_method 压缩方法，提供三种：NONE，LZ77，HUFFMAN
     * @param encryption_method 加密方法，提供两种：NONE，AES_256_CBC
     * @param password 加密用的密码
     * @param not_including_files 不需要打包的多个文件，用换行分割，传相对路径，相对路径是相对于source的
     * @return 两种返回值，一是“OK”，表示没有问题；二是报错信息。所有不是“OK”的都是有问题的，报错信息在返回值里。
     */
    std::string back_up(const std::filesystem::path& source,
                        const std::filesystem::path& destination,
                        const std::string& compression_method,
                        const std::string& encryption_method,
                        const std::string& password,
                        const std::string& not_including_files);

    /**
     * @brief 获取指定目录的备份包的信息
     * @param path 指定目录
     * @return 两种返回值，一是错误信息，表示这个文件没法解析或者解析出错，若非错误则是相关信息，
     * 形如“key: value”的形式进行展示，换行符分割。
     */
    std::string info(const std::filesystem::path& path);

    /**
     * @brief 将指定的备份包还原到指定目录
     * @param source 备份包的目录
     * @param destination 需要还原的指定位置，即展开备份包的路径
     * @param password 解密用的密钥，如果没有加密这个就为空
     * @return 两种返回值，一是“OK”，表示没有问题；二是报错信息。所有不是“OK”的都是有问题的，报错信息在返回值里。
     */
    std::string restore_backup(const std::filesystem::path& source,
                               const std::filesystem::path& destination,
                               const std::string& password);
}


#endif //DATA_BACK_UP_BACK_UP_H