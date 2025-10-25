//
// Created by hyh on 2025/10/16.
//

#include "../../include/compression_method/lz77.h"

namespace
{
    /**
     * @brief 获取串的最长前缀匹配
     * @param pattern 模式串
     * @return 前缀匹配数量
     */
    std::vector<int> get_prefix(const std::vector<char>& pattern)
    {
        std::vector<int> result(pattern.size(),0);
        for (unsigned int i = 1; i < pattern.size(); i++)
        {
            auto j = result[i-1];
            while (pattern[i] != pattern[j] && j >0)
            {
                j = result[j-1];
            }
            if (pattern[i] == pattern[j])
            {
                j++;
            }
            result[i] = j;
        }
        return result;
    }
}

std::tuple<unsigned int, unsigned int, char> data_packet::detail::compression_tuple(const std::vector<char>& data,
    const std::vector<char>& pattern, const char last)
{
    std::tuple<unsigned int, unsigned int, char> result{0,0,pattern.front()};

    auto next = get_prefix(pattern);

    size_t j = 0;

    for (size_t i = 0; i < data.size(); i++)
    {
        while (j > 0 && j < pattern.size() && data.at(i) != pattern.at(j))
        {
            j = next[j-1];
        }

        if (j >= pattern.size())
        {
            j = next[j-1];
        }

        if (pattern.at(j) == data.at(i))
        {
            ++j;
        }

        if (j >= std::get<1>(result))
        {
            std::get<1>(result) = j;
            std::get<0>(result) = data.size() - i + j - 1;
            std::get<2>(result) = j < pattern.size()? pattern.at(j) : last;
        }
    }

    return result;
}