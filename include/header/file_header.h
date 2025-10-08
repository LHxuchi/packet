//
// Created by hyh on 2025/10/2.
//

#ifndef DATA_BACK_UP_FILE_HEADER_H
#define DATA_BACK_UP_FILE_HEADER_H

#include "header/header.h"
#include "../utils/byte_conversion.h"

namespace data_packet
{
    /**
     * @class file_header
     * @brief 文件头类，继承自基础header类，用于存储备份文件的整体元数据信息
     *
     * 该类负责管理备份数据包的全局信息，包括版本号、创建时间、文件数量统计、
     * 总大小信息以及校验相关数据。提供序列化和反序列化功能，用于在存储或传输
     * 过程中处理文件头信息。
     *
     * 此类设计为final，不允许被继承。
     */
    class file_header final : public header
    {
    public:
        /**
         * @brief 默认构造函数，初始化所有成员变量为默认值
         */
        file_header() = default;

        /**
         * @brief 析构函数，覆盖基类析构函数
         */
        ~file_header() override = default;

        /**
         * @brief 获取file_header的序列化数据
         * @return 序列化的buffer，包含所有文件头信息
         */
        std::unique_ptr<char[]> get_buffer() override;

        /**
         * @brief 从序列化数据反构造file_header
         * @param data 指定序列化数据起始位置的指针
         */
        void set_buffer(const char* data) override;

        /**
         * @brief 获取文件头的大小
         * @return 文件头的大小(字节数)
         */
        size_t header_size() override;

        /**
         * @brief 获取文件格式版本号
         * @return 版本号(双字节)
         */
        [[nodiscard]] word get_version() const;

        /**
         * @brief 设置文件格式版本号
         * @param other_version 要设置的版本号
         */
        void set_version(word other_version);

        /**
         * @brief 获取整个数据包的CRC32校验值
         * @return CRC32校验值(四字节)
         */
        [[nodiscard]] dword get_crc_32() const;

        /**
         * @brief 设置整个数据包的CRC32校验值
         * @param other_crc_32 要设置的CRC32校验值
         */
        void set_crc_32(dword other_crc_32);

        /**
         * @brief 获取数据包的创建时间
         * @return 时间戳(八字节，通常为自纪元以来的秒数或毫秒数)
         */
        [[nodiscard]] qword get_creation_time() const;

        /**
         * @brief 刷新数据包的创建时间为当前系统时间
         */
        void refresh_creation_time();

        /**
         * @brief 获取数据包中包含的文件数量
         * @return 文件数量(四字节)
         */
        [[nodiscard]] dword get_file_number() const;

        /**
         * @brief 设置数据包中包含的文件数量
         * @param other_file_number 要设置的文件数量
         */
        void set_file_number(dword other_file_number);

        /**
         * @brief 获取处理后(压缩/加密)的数据包总大小
         * @return 处理后的数据大小(字节数，八字节)
         */
        [[nodiscard]] qword get_file_size() const;

        /**
         * @brief 设置处理后(压缩/加密)的数据包总大小
         * @param other_file_size 要设置的处理后数据大小
         */
        void set_file_size(qword other_file_size);

        /**
         * @brief 获取原始(未处理)的数据包总大小
         * @return 原始数据大小(字节数，八字节)
         */
        [[nodiscard]] qword get_original_file_size() const;

        /**
         * @brief 设置原始(未处理)的数据包总大小
         * @param other_original_file_size 要设置的原始数据大小
         */
        void set_original_file_size(qword other_original_file_size);

        /**
         * @brief 获取文件头的校验和
         * @return 校验和值(四字节)
         */
        [[nodiscard]] dword get_checksum() const;

        /**
         * @brief 刷新文件头的校验和(重新计算)
         */
        void refresh_checksum();

    private:
        byte version[2]{0};               ///< 文件格式版本号，2字节
        byte creation_time[8]{0};         ///< 数据包创建时间戳，8字节
        byte file_number[4]{0};           ///< 包含的文件数量，4字节
        byte file_size[8]{0};             ///< 处理后总大小，8字节
        byte original_file_size[8]{0};    ///< 原始总大小，8字节
        byte checksum[4]{0};              ///< 文件头校验和，4字节
        byte crc_32[4]{0};                ///< 整个数据包的CRC32校验值，4字节

        /**
         * @brief 文件头固定部分的总大小
         *
         * 计算方式为所有私有成员变量的大小之和，用于序列化和反序列化时的缓冲区大小计算
         */
        static constexpr size_t SIZE = sizeof(version) +
            sizeof(creation_time) + sizeof(file_number) +
            sizeof(file_size) + sizeof(original_file_size) +
            sizeof(checksum) + sizeof(crc_32);
    };
} // data_packet

#endif //DATA_BACK_UP_FILE_HEADER_H