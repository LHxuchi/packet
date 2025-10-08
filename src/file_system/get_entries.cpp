//
// Created by hyh on 2025/10/6.
//

#include "../../include/file_system/get_entries.h"

#include <queue>

namespace
{
    // 检查是否有读取权限
    bool has_read_permission(const std::filesystem::path& path) {
        namespace fs = std::filesystem;
        try {
            // 获取文件权限
            auto perms = fs::status(path).permissions();

            // 检查所有者、组或其他用户是否有读权限
            return (perms & fs::perms::owner_read) != fs::perms::none ||
                   (perms & fs::perms::group_read) != fs::perms::none ||
                   (perms & fs::perms::others_read) != fs::perms::none;
        } catch (const fs::filesystem_error& e) {
            return false;
        }
    }
}

auto data_packet::get_entries(const std::filesystem::path& path) -> std::set<std::filesystem::directory_entry>
{
    namespace fs = std::filesystem;

    if (!fs::directory_entry(path).exists())
    {
        return {};
    }

    if (!has_read_permission(path))
    {
        throw std::runtime_error("File does not have read permission");
    }

    // 指定目录迭代器
    fs::directory_iterator dir_it(path);

    // 最终结果
    std::set<std::filesystem::directory_entry> entries;

    // 采用层序遍历，使用队列实现
    std::queue<std::filesystem::directory_iterator> entries_queue;

    entries_queue.push(dir_it);

    while (!entries_queue.empty())
    {
        for (const auto& entry : entries_queue.front())
        {
            if (entry.is_directory())
            {
                entries_queue.emplace(entry.path());
            }
            entries.emplace(entry);
        }
        entries_queue.pop();
    }

    return entries;
}
