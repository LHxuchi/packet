//
// Created by hyh on 2025/10/6.
//

#ifndef DATA_BACK_UP_GET_ENTRIES_H
#define DATA_BACK_UP_GET_ENTRIES_H

#include <filesystem>
#include <set>
#include <functional>

namespace data_packet
{

    /* 没法处理隐藏文件 */
    /*
    class ordered_by_inode
    {
    public:
        bool operator()(const std::filesystem::directory_entry& a,const std::filesystem::directory_entry& b) const
        {
            return std::filesystem::equivalent(a.path().filename(),b.path().filename());
        }
    };
    */

    /**
     * @brief 根据输入的指定路径输出路径下所有文件（包括隐藏文件）
     * @param path 指定路径
     * @return 文件集合
     */
    auto get_entries(const std::filesystem::path& path) -> std::multiset<std::filesystem::directory_entry>;

    using filter_t = std::function<bool(const std::filesystem::directory_entry&)>;

    /**
     * @brief 根据输入的指定路径输出路径下筛选过的所有文件
     * @param path 指定路径
     * @param filter 过滤器
     * @return 文件集合
     */
    auto get_entries(const std::filesystem::path& path,
                     const std::function<bool(const std::filesystem::directory_entry&)>& filter)
    -> std::multiset<std::filesystem::directory_entry>;

    /**
     * @brief 判断当前路径是否为硬链接
     * @param path 指定路径
     * @return true 是硬链接， false 不是硬链接
     */
    bool is_hard_link(const std::filesystem::path& path);
}
#endif //DATA_BACK_UP_GET_ENTRIES_H
