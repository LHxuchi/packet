//
// Created by hyh on 2025/10/6.
//

#include "../../include/file_system/get_entries.h"

#include <queue>

bool data_packet::is_hard_link(const std::filesystem::path& path)
{
    const std::filesystem::directory_entry entry(path);
    return !entry.is_directory()&&entry.hard_link_count() >= 2;
}

/* 加个filter */
auto data_packet::get_entries(const std::filesystem::path& path) -> std::multiset<std::filesystem::directory_entry>
{
    namespace fs = std::filesystem;

    if (!fs::directory_entry(path).exists())
    {
        return {};
    }

    // 指定目录迭代器
    fs::directory_iterator dir_it(path);

    // 最终结果
    decltype(get_entries(path)) entries;

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

auto data_packet::get_entries(const std::filesystem::path& path,
    const std::function<bool(const std::filesystem::directory_entry&)>& filter) -> std::multiset<std::filesystem::
    directory_entry>
{
    auto entries = get_entries(path);
    for (auto it = entries.begin(); it != entries.end(); ++it)
    {
        if (filter(*it))
        {
            it = entries.erase(it);
        }
    }

    return entries;
}
