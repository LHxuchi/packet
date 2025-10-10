//
// Created by hyh on 2025/10/8.
//

#ifndef DATA_BACK_UP_LOCAL_PACKET_H
#define DATA_BACK_UP_LOCAL_PACKET_H

#include <array>

#include "../header/local_file_header.h"

namespace data_packet
{
    /**
     * @class local_packet
     * @brief 本地文件包，包文件中每个分文件存储位置
     */
    class local_packet
    {
    public:
        /**
         * @brief 默认构造函数
         */
        local_packet()
        {
            _header = std::make_unique<local_file_header>();
        }

        /**
         * @brief 默认析构函数
         */
        ~local_packet() = default;

        /**
         * @brief 复制拷贝函数，禁止该操作
         * @param other 其他local_packet
         */
        local_packet(const local_packet& other) = delete;

        /**
         * @brief 移动拷贝函数，仅允许移动语义进行资源转移
         * @param other 其他local_packet
         */
        local_packet(local_packet&& other) noexcept
            : _header(std::move(other._header)),
              _data(std::move(other._data))
        {
        }

        /**
         * @brief 显式复制拷贝函数，禁止该操作
         * @param other 其他local_packet
         * @return *this
         */
        local_packet& operator=(const local_packet& other) = delete;

        /**
         * @brief 显式移动拷贝函数，仅允许移动语义进行资源转移
         * @param other 其他local_packet
         * @return *this
         */
        local_packet& operator=(local_packet&& other) noexcept
        {
            if (this == &other)
                return *this;
            _header = std::move(other._header);
            _data = std::move(other._data);
            return *this;
        }

        /**
         * @brief 获取当前包文件数据流
         * @return 包文件数据流，不可更改当前权限
         */
        [[nodiscard]] const std::unique_ptr<byte[]>& get_data() const
        {
            return _data;
        }

        /**
         * @brief 设置新的包文件数据流
         * @param data 指定数据流
         */
        void set_data(std::unique_ptr<byte[]>&& data)
        {
            _data.reset(data.get());
            data.release();
        }

        /**
         * @brief local_file_header序列化函数
         * @param buffer 指定需要序列化的数据首部
         */
        void set_header_buffer(const byte* buffer)
        {
            _header->set_buffer(buffer);
        }

        /**
         * @brief 设置文件所有者的用户ID(UID)
         * @param uid 要设置的用户ID值
         * @note 直接转发到底层local_file_header的对应方法
         */
        void set_uid(uint32_t uid) { _header->set_uid(uid); }

        /**
         * @brief 设置文件所有者的组ID(GID)
         * @param gid 要设置的组ID值
         * @note 直接转发到底层local_file_header的对应方法
         */
        void set_gid(uint32_t gid) { _header->set_gid(gid); }

        /**
         * @brief 设置文件所有者的用户名
         * @param uname 要设置的用户名字符串
         * @note 直接转发到底层local_file_header的对应方法
         */
        void set_uname(const std::string& uname) { _header->set_uname(uname); }

        /**
         * @brief 设置文件所有者的组名
         * @param gname 要设置的组名字符串
         * @note 直接转发到底层local_file_header的对应方法
         */
        void set_gname(const std::string& gname) { _header->set_gname(gname); }

        /**
         * @brief 设置文件的最后修改时间
         * @param time 要设置的时间戳(自纪元以来的秒数)
         * @note 直接转发到底层local_file_header的对应方法
         */
        void set_last_modification_time(uint64_t time) { _header->set_last_modification_time(time); }

        /**
         * @brief 设置文件的最后访问时间
         * @param time 要设置的时间戳(自纪元以来的秒数)
         * @note 直接转发到底层local_file_header的对应方法
         */
        void set_last_access_time(uint64_t time) { _header->set_last_access_time(time); }

        /**
         * @brief 设置文件的权限信息
         * @param perms 要设置的文件权限对象(std::filesystem::perms)
         * @note 直接转发到底层local_file_header的对应方法
         */
        void set_permissions(const std::filesystem::perms& perms) { _header->set_permissions(perms); }

        /**
         * @brief 设置文件类型
         * @param type 要设置的文件类型(std::filesystem::file_type)
         * @note 直接转发到底层local_file_header的对应方法
         */
        void set_file_type(const std::filesystem::file_type& type) { _header->set_file_type(type); }

        /**
         * @brief 设置文件使用的压缩方法
         * @param method 要设置的压缩方法枚举值
         * @note 直接转发到底层local_file_header的对应方法
         */
        void set_compression_method(local_file_header::compression_method method)
        {
            _header->set_compression_method(method);
        }

        /**
         * @brief 设置文件使用的加密方法
         * @param method 要设置的加密方法枚举值
         * @note 直接转发到底层local_file_header的对应方法
         */
        void set_encryption_method(local_file_header::encryption_method method)
        {
            _header->set_encryption_method(method);
        }

        /**
         * @brief 设置加密使用的盐值
         * @param salt 16字节的盐值数组
         * @note 直接转发到底层local_file_header的对应方法
         */
        void set_salt(const std::array<uint8_t, 16>& salt) { _header->set_salt(salt); }

        /**
         * @brief 设置链接名的长度
         * @param length 链接名的长度(字节数)
         * @note 直接转发到底层local_file_header的对应方法
         */
        void set_link_name_length(uint16_t length) { _header->set_link_name_length(length); }

        /**
         * @brief 设置原始文件的大小(压缩或加密前)
         * @param size 原始文件大小(字节数)
         * @note 直接转发到底层local_file_header的对应方法
         */
        void set_original_file_size(uint64_t size) { _header->set_original_file_size(size); }

        /**
         * @brief 设置处理后文件的大小(压缩或加密后)
         * @param size 处理后文件的大小(字节数)
         * @note 直接转发到底层local_file_header的对应方法
         */
        void set_file_size(uint64_t size) { _header->set_file_size(size); }

        /**
         * @brief 设置文件名的长度
         * @param length 文件名的长度(字节数)
         * @note 直接转发到底层local_file_header的对应方法
         */
        void set_file_name_length(uint16_t length) { _header->set_file_name_length(length); }

        /**
         * @brief 设置链接名
         * @param name 链接名字符串
         * @note 直接转发到底层local_file_header的对应方法
         */
        void set_link_name(const std::string& name) { _header->set_link_name(name); }

        /**
         * @brief 设置文件名
         * @param name 文件名字符串
         * @note 直接转发到底层local_file_header的对应方法
         */
        void set_file_name(const std::string& name) { _header->set_file_name(name); }

        /**
         * @brief 根据当前文件头数据刷新校验和
         */
        void refresh_checksum() { _header->refresh_checksum(); }

        /**
         * @brief 根据当前系统时间刷新时间戳
         */
        void refresh_creation_time() { _header->refresh_creation_time(); }

        /**
         * @brief 根据当前文件流数据与文件长度刷新crc校验码
         */
        void refresh_crc_32();

        [[nodiscard]] const local_file_header& info() const { return *_header; }

    private:
        std::unique_ptr<local_file_header> _header;
        std::unique_ptr<byte[]> _data{nullptr};
    };
} // data_packet

#endif //DATA_BACK_UP_LOCAL_PACKET_H
