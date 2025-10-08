//
// Created by hyh on 2025/10/6.
//

#ifndef DATA_BACK_UP_GET_ENTRIES_H
#define DATA_BACK_UP_GET_ENTRIES_H
#include <filesystem>
#include <set>

namespace data_packet
{
    /**
     * @brief 根据输入的指定路径输出路径下所有文件（包括隐藏文件）
     * @param path 指定路径
     * @return 文件集合
     */
    auto get_entries(const std::filesystem::path& path) -> std::multiset<std::filesystem::directory_entry>;

    /**
     * @brief 判断当前路径是否为硬链接
     * @param path 指定路径
     * @return true 是硬链接， false 不是硬链接
     */
    bool is_hard_link(const std::filesystem::path& path);
}
#endif //DATA_BACK_UP_GET_ENTRIES_H
