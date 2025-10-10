//
// Created by hyh on 2025/10/8.
//

#ifndef DATA_BACK_UP_PACKET_H
#define DATA_BACK_UP_PACKET_H

#include <vector>

#include "../header/file_header.h"
#include "../local_packet/local_packet.h"

namespace data_packet
{
    class packet
    {
    public:
        /**
         * @brief 默认构造
         */
        packet() = default;

        /**
         * @brief 默认析构
         */
        ~packet() = default;

        packet(const packet& other) = delete;

        /**
         * @brief 默认移动
         * @param other
         */
        packet(packet&& other) noexcept
            : _header(other._header),
              _packets(std::move(other._packets)){}

        packet& operator=(const packet& other) = delete;

        /**
         * @brief 默认移动
         * @param other
         * @return *this
         */
        packet& operator=(packet&& other) noexcept
        {
            if (this == &other)
                return *this;
            _header = other._header;
            _packets = std::move(other._packets);
            return *this;
        }

        /**
         * @brief 获取当前包文件组
         * @return 包文件组
         */
        [[nodiscard]] std::vector<local_packet>& packets() { return _packets; }

        /**
         * @brief 获取当前header中的信息
         * @return this->_header
         */
        [[nodiscard]] const file_header& info() const {return _header;}

        /**
         * @brief 设置当前文件版本号
         * @param version 版本号
         */
        void set_version(word version){_header.set_version(version);}

        /**
         * @brief 根据当前系统时间刷新创建时间
         */
        void refresh_creation_time(){_header.refresh_creation_time();}

        /**
         * @brief 根据当前文件数量刷新文件数量字段
         */
        void refresh_file_number(){_header.set_file_number(_packets.size());}

        /**
         * @brief 根据当前包文件组的文件大小刷新当前总文件大小（包含包文件头与本文件头）
         */
        void refresh_file_size();

        /**
         * @brief 根据当前包文件组的原始文件大小刷新当前总原始文件大小（包含包文件头与本文件头）
         */
        void refresh_original_file_size();

        /**
         * @brief 根据当前头字段刷新校验和
         */
        void refresh_checksum(){_header.refresh_checksum();}

        /**
         * @brief 根据包文件的CRC字段计算得到总文件的CRC字段
         */
        void refresh_crc_32();

        /**
         * @brief 文件头序列化
         * @param buffer 指定序列化数据起始位置
         */
        void set_header_buffer(const char* buffer){_header.set_buffer(buffer);}



    private:
        file_header _header{};
        std::vector<local_packet> _packets{};
    };

    /**
     * @brief 将指定路径下的文件打包
     * @param path 指定路径
     * @return 文件包
     */
    packet make_packet(const std::filesystem::path& path);

} // data_packet

#endif //DATA_BACK_UP_PACKET_H