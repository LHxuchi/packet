//
// Created by hyh on 2025/9/30.
//

#ifndef DATA_BACK_UP_HEADER_H
#define DATA_BACK_UP_HEADER_H
#include <memory>

namespace data_packet
{

    /**
     * @brief 头接口，指明文件头必须实现的功能。
     */
    class header{
    public:

        virtual ~header() = default;

        /**
         * @brief 获取文件头数据流
         * @return
         */
        virtual std::unique_ptr<char[]> get_buffer() = 0;

        /**
         * @brief 根据输入数据流设置文件头
         * @param data 指定的数据流
         * @param size 数据流长度
         */
        virtual void set_buffer(const char* data) = 0;

        /**
         * @brief 获取当前文件头长度
         * @return 当前文件头长度
         */
        virtual size_t header_size() = 0;

    };

}


#endif //DATA_BACK_UP_HEADER_H
