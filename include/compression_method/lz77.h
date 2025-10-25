//
// Created by hyh on 2025/10/14.
//

#ifndef DATA_BACK_UP_LZ77_H
#define DATA_BACK_UP_LZ77_H
#include <chrono>
#include <cstring>
#include <memory>
#include <vector>

#include "utils/byte_conversion.h"

namespace data_packet
{
    constexpr unsigned short FRONT_SIZE = 255;
    constexpr unsigned short BACK_SIZE = 65535;
    namespace detail
    {
        /**
         * @brief 找到前缓冲区中与扫描缓冲区的最长匹配串位置，并返回扫描缓冲区已匹配串的下一个字符
         * @param data 前缓冲区
         * @param pattern 扫描缓冲区
         * @param last 匹配结束后最后一个字符
         * @return 匹配位置，匹配长度，下一个字符
         */
        std::tuple<unsigned int, unsigned int,char> compression_tuple(
            const std::vector<char>& data, const std::vector<char>& pattern, char last);

    }

    /**
     * @brief 压缩算法，返回压缩后的数据
     * @tparam Iter 前向迭代器
     * @param begin 压缩数据开始闭区间
     * @param end 压缩数据结束开区间
     * @return First: 压缩后数据 Second: 压缩后数据长度
     */
    template<typename Iter>
    std::pair<std::unique_ptr<byte[]>, size_t> lz77_compress(const Iter& begin, const Iter& end)
    {
        static_assert(std::is_same_v<typename std::iterator_traits<Iter>::value_type,char>,
            "Iterator value_t must be type 'char'.");

        // 分割点
        Iter separator = begin;

        int front_buffer_size = 0, // 前缓冲区大小
            back_buffer_size = 0;  // 后缓冲区大小

        Iter pattern_end = separator,  // 后缓冲区结束位置
            content_begin = separator; // 前缓冲区开始位置

        // 初始化前缓冲区
        while (pattern_end != end && front_buffer_size < FRONT_SIZE)
        {
            ++pattern_end;
            ++front_buffer_size;
        }

        std::vector<std::tuple<unsigned int,unsigned int,char>> results;

        bool need_padding = true;

        while (separator != end)
        {
            // 压缩信息
            auto info = detail::compression_tuple(
                {content_begin,separator},{separator,pattern_end} , '\0');

            results.emplace_back(info);

            // 更新分界点，前缓冲区大小与后缓冲区大小
            for (unsigned int i = 0; i <= std::get<1>(info); ++i)
            {
                if (separator == end)
                {
                    need_padding = false;
                    break;
                }
                ++separator;
                --front_buffer_size;
                ++back_buffer_size;
            }

            // 更新后缓冲区起点
            for (;back_buffer_size > BACK_SIZE; --back_buffer_size)
            {
                if (content_begin == separator)
                    break;
                ++content_begin;
            }

            // 更新前缓冲区终点
            for (;front_buffer_size < FRONT_SIZE; ++front_buffer_size)
            {
                if (pattern_end == end)
                    break;
                ++pattern_end;
            }
        }

        if (need_padding)
        {
            results.emplace_back(0,0,'\0');
        }

        auto ret = std::make_unique<byte[]>(results.size() * 4);
        byte* write_posi = ret.get();

        for (const auto& [posi,match,last] : results)
        {
            auto posi_bytes = to_bytes(posi);
            auto match_bytes = to_bytes(match);

            *write_posi = std::get<2>(posi_bytes);
            *(write_posi + 1) = std::get<3>(posi_bytes);
            write_posi += 2;

            *write_posi = std::get<3>(match_bytes);
            write_posi += 1;

            *write_posi = last;
            write_posi += 1;
        }

        return std::make_pair(std::move(ret), results.size() * 4);
    }

    /**
     * @brief 对lz77压缩的数据解压
     * @tparam Iter 输入迭代器
     * @param begin 解压初始位置闭区间
     * @param end 解压结束位置开区间
     * @return 解压数据与解压数据大小
     */
    template<typename Iter>
    std::pair<std::unique_ptr<byte[]>,size_t> lz77_decompress(const Iter& begin, const Iter& end)
    {
        static_assert(std::is_same_v<typename std::iterator_traits<Iter>::value_type,char>,
            "Iterator value_t must be type 'char'.");

        auto it = begin;

        std::vector<byte> sequence;

        while (it != end)
        {
            byte bytes[4];
            for (unsigned short i = 0;i<4&& it!= end;++i)
            {
                bytes[i] = *it;
                ++it;
            }

            unsigned short back_posi = make_word({bytes[0],bytes[1]});
            unsigned char front_posi = bytes[2];
            char last = bytes[3];

            sequence.insert(sequence.end(), sequence.end() - back_posi, sequence.end() - back_posi + front_posi);
            sequence.emplace_back(last);
        }

        sequence.pop_back();

        auto ret = std::make_unique<byte[]>(sequence.size());
        memcpy(ret.get(), sequence.data(), sequence.size());

        return std::make_pair(std::move(ret), sequence.size());
    }

}

#endif //DATA_BACK_UP_LZ77_H