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
    /**
     * @class local_file_header
     * @brief 本地文件头类，继承自基础header类，用于存储单个文件的元数据信息
     *
     * 该类负责管理备份数据包中单个文件的详细信息，包括文件属性、权限、时间戳、
     * 压缩方式、加密信息等元数据。提供了序列化和反序列化功能，可将文件元数据
     * 转换为字节流以便存储或传输，也可从字节流中恢复文件元数据。
     *
     * 支持文件所有权信息(UID/GID/用户名/组名)、时间信息(创建/修改/访问时间)、
     * 校验信息(CRC32/校验和)、压缩与加密设置等文件属性的管理。
     */
    class local_file_header final : public header
    {
    public:
        /**
         * @enum compression_method
         * @brief 压缩算法枚举，定义支持的文件压缩方式
         */
        enum class compression_method
        {
            None = 0,  ///< 无压缩
            LZ77       ///< 使用LZ77压缩算法
        };

        /**
         * @enum encryption_method
         * @brief 加密算法枚举，定义支持的文件加密方式
         */
        enum class encryption_method
        {
            None = 0,   ///< 不加密
            my_method   ///< 自定义加密方法
        };

        /**
         * @brief 默认构造函数，初始化所有成员变量为默认值
         */
        local_file_header() = default;

        /**
         * @brief 析构函数，覆盖基类析构函数
         */
        ~local_file_header() override = default;

        /**
         * @brief 将文件头信息序列化为字节缓冲区
         * @return 包含文件头信息的唯一指针字节数组
         */
        std::unique_ptr<char[]> get_buffer() override;

        /**
         * @brief 从字节缓冲区反序列化文件头信息
         * @param data 包含文件头信息的字节缓冲区
         */
        void set_buffer(const char* data) override;

        /**
         * @brief 获取文件头的大小
         * @return 文件头的大小(字节数)
         */
        size_t header_size() override;

        /**
         * @brief 获取文件所有者的用户ID(UID)
         * @return UID值
         */
        [[nodiscard]] uint32_t get_uid() const;

        /**
         * @brief 设置文件所有者的用户ID(UID)
         * @param uid 要设置的UID值
         */
        void set_uid(uint32_t uid);

        /**
         * @brief 获取文件所有者的组ID(GID)
         * @return GID值
         */
        [[nodiscard]] uint32_t get_gid() const;

        /**
         * @brief 设置文件所有者的组ID(GID)
         * @param gid 要设置的GID值
         */
        void set_gid(uint32_t gid);

        /**
         * @brief 获取文件所有者的用户名
         * @return 用户名字符串
         */
        [[nodiscard]] std::string get_uname() const;

        /**
         * @brief 设置文件所有者的用户名
         * @param uname 要设置的用户名
         */
        void set_uname(const std::string& uname);

        /**
         * @brief 获取文件所有者的组名
         * @return 组名字符串
         */
        [[nodiscard]] std::string get_gname() const;

        /**
         * @brief 设置文件所有者的组名
         * @param gname 要设置的组名
         */
        void set_gname(const std::string& gname);

        /**
         * @brief 获取文件的创建时间
         * @return 时间戳(通常为自纪元以来的秒数或毫秒数)
         */
        [[nodiscard]] uint64_t get_creation_time() const;

        /**
         * @brief 刷新文件的创建时间为当前时间
         */
        void refresh_creation_time();

        /**
         * @brief 获取文件的最后修改时间
         * @return 时间戳(通常为自纪元以来的秒数或毫秒数)
         */
        [[nodiscard]] uint64_t get_last_modification_time() const;

        /**
         * @brief 设置文件的最后修改时间
         * @param last_modification_time 要设置的时间戳
         */
        void set_last_modification_time(uint64_t last_modification_time);

        /**
         * @brief 获取文件的最后访问时间
         * @return 时间戳(通常为自纪元以来的秒数或毫秒数)
         */
        [[nodiscard]] uint64_t get_last_access_time() const;

        /**
         * @brief 设置文件的最后访问时间
         * @param last_access_time 要设置的时间戳
         */
        void set_last_access_time(uint64_t last_access_time);

        /**
         * @brief 获取文件的权限信息
         * @return 文件权限对象(std::filesystem::perms)
         */
        [[nodiscard]] std::filesystem::perms get_permissions() const;

        /**
         * @brief 设置文件的权限信息
         * @param permissions 要设置的文件权限
         */
        void set_permissions(const std::filesystem::perms& permissions);

        /**
         * @brief 获取文件类型
         * @return 文件类型对象(std::filesystem::file_type)
         */
        [[nodiscard]] std::filesystem::file_type get_file_type() const;

        /**
         * @brief 设置文件类型
         * @param file_type 要设置的文件类型
         */
        void set_file_type(const std::filesystem::file_type& file_type);

        /**
         * @brief 获取文件内容的CRC32校验值
         * @return CRC32校验值
         */
        [[nodiscard]] uint32_t get_crc_32() const;

        /**
         * @brief 设置文件内容的CRC32校验值
         * @param crc_32 要设置的CRC32校验值
         */
        void set_crc_32(uint32_t crc_32);

        /**
         * @brief 获取文件头的校验和
         * @return 校验和值
         */
        [[nodiscard]] uint32_t get_checksum() const;

        /**
         * @brief 刷新文件头的校验和(重新计算)
         */
        void refresh_checksum();

        /**
         * @brief 获取文件使用的压缩方法
         * @return 压缩方法枚举值
         */
        [[nodiscard]] compression_method get_compression_method() const;

        /**
         * @brief 设置文件使用的压缩方法
         * @param compression_method 要设置的压缩方法
         */
        void set_compression_method(compression_method compression_method);

        /**
         * @brief 获取文件使用的加密方法
         * @return 加密方法枚举值
         */
        [[nodiscard]] encryption_method get_encryption_method() const;

        /**
         * @brief 设置文件使用的加密方法
         * @param encryption_method 要设置的加密方法
         */
        void set_encryption_method(encryption_method encryption_method);

        /**
         * @brief 获取加密使用的盐值
         * @return 16字节的盐值数组
         */
        [[nodiscard]] std::array<uint8_t, 16> get_salt() const;

        /**
         * @brief 设置加密使用的盐值
         * @param salt 16字节的盐值数组
         */
        void set_salt(const std::array<uint8_t, 16>& salt);

        /**
         * @brief 获取链接名的长度
         * @return 链接名长度(字节数)
         */
        [[nodiscard]] uint16_t get_link_name_length() const;

        /**
         * @brief 设置链接名的长度
         * @param link_name_length 链接名长度(字节数)
         */
        void set_link_name_length(uint16_t link_name_length);

        /**
         * @brief 获取原始文件的大小(压缩或加密前)
         * @return 原始文件大小(字节数)
         */
        [[nodiscard]] uint64_t get_original_file_size() const;

        /**
         * @brief 设置原始文件的大小(压缩或加密前)
         * @param original_file_size 原始文件大小(字节数)
         */
        void set_original_file_size(uint64_t original_file_size);

        /**
         * @brief 获取处理后文件的大小(压缩或加密后)
         * @return 处理后文件大小(字节数)
         */
        [[nodiscard]] uint64_t get_file_size() const;

        /**
         * @brief 设置处理后文件的大小(压缩或加密后)
         * @param file_size 处理后文件大小(字节数)
         */
        void set_file_size(uint64_t file_size);

        /**
         * @brief 获取文件名的长度
         * @return 文件名长度(字节数)
         */
        [[nodiscard]] uint16_t get_file_name_length() const;

        /**
         * @brief 设置文件名的长度
         * @param file_name_length 文件名长度(字节数)
         */
        void set_file_name_length(uint16_t file_name_length);

        /**
         * @brief 获取链接名
         * @return 链接名字符串
         */
        [[nodiscard]] std::string get_link_name() const;

        /**
         * @brief 设置链接名
         * @param link_name 链接名字符串
         */
        void set_link_name(const std::string& link_name);

        /**
         * @brief 获取文件名
         * @return 文件名字符串
         */
        [[nodiscard]] std::string get_file_name() const;

        /**
         * @brief 设置文件名
         * @param file_name 文件名字符串
         */
        void set_file_name(const std::string& file_name);

    private:
        byte uid_[4]{0};                     ///< 用户ID(UID)，4字节
        byte gid_[4]{0};                     ///< 组ID(GID)，4字节
        byte uname_[32]{0};                  ///< 用户名，32字节
        byte gname_[32]{0};                  ///< 组名，32字节
        byte creation_time_[8]{0};           ///< 创建时间戳，8字节
        byte last_modification_time_[8]{0};  ///< 最后修改时间戳，8字节
        byte last_access_time_[8]{0};        ///< 最后访问时间戳，8字节
        byte file_type_and_permissions_[2]{0};///< 文件类型和权限，2字节
        byte crc_32_[4]{0};                  ///< CRC32校验值，4字节
        byte checksum_[4]{0};                ///< 校验和，4字节
        byte compression_and_encryption_{0}; ///< 压缩和加密方式，1字节
        byte salt_[16]{0};                   ///< 加密盐值，16字节
        byte original_file_size_[8]{0};      ///< 原始文件大小，8字节
        byte file_size_[8]{0};               ///< 处理后文件大小，8字节
        byte link_name_length_[2]{0};        ///< 链接名长度，2字节
        byte file_name_length_[2]{0};        ///< 文件名长度，2字节

        std::unique_ptr<byte[]> link_name_{nullptr}; ///< 链接名数据
        std::unique_ptr<byte[]> file_name_{nullptr}; ///< 文件名数据

        static constexpr byte compression_method_mask = static_cast<byte>(0xf0); ///< 压缩方法掩码
        static constexpr byte encryption_method_mask = 0xf;                      ///< 加密方法掩码
        static constexpr word permission_mask = 0x1ff;                           ///< 权限掩码
        static constexpr byte file_type_mask = static_cast<byte>(0xfe);          ///< 文件类型掩码

    public:
        /**
         * @brief 固定部分的大小(不包含变长的文件名和链接名)
         */
        static constexpr size_t SIZE = sizeof(uid_) + sizeof(gid_) + sizeof(uname_) +
            sizeof(gname_) + sizeof(creation_time_) + sizeof(last_modification_time_) +
                sizeof(last_access_time_) + sizeof(file_type_and_permissions_) + sizeof(crc_32_)+
                    sizeof(checksum_) + sizeof(compression_and_encryption_) + sizeof(salt_) +
                        sizeof(original_file_size_) + sizeof(file_size_) + sizeof(link_name_length_) +
                            + sizeof(file_name_length_);

        /**
         * @brief 最大可能的大小(包含最大长度的文件名和链接名)
         */
        static constexpr size_t MAX_SIZE = SIZE + 0xffff + 0xffff;
    };
} // data_packet

#endif //DATA_BACK_UP_LOCAL_FILE_HEADER_H