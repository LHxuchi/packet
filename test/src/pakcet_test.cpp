//
// Created by hyh on 2025/10/10.
//

#include <set>
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_session.hpp>

#include "../../include/packet/packet.h"

TEST_CASE("verify packet content and structure", "[packet]")
{
    namespace dp = data_packet;
    namespace fs = std::filesystem;

    auto test_path = fs::path("/home/hyh/CLionProjects/packet/mytest");
    auto pkt = dp::make_packet(test_path);
    auto& packets = pkt.packets();
    auto& header = pkt.info();

    // 验证文件总数（与现有测试呼应，确认目录结构解析正确）
    CHECK(header.get_file_number() == 10);

    // 验证基本头信息有效性
    CHECK(header.get_version() != 0);  // 假设版本号已正确设置
    CHECK(header.get_creation_time() != 0);  // 验证创建时间已刷新
    CHECK(header.get_checksum() != 0);  // 验证校验和已计算
    CHECK(header.get_crc_32() != 0);    // 验证CRC32已计算

    // 验证总文件大小和原始文件大小为正数
    CHECK(header.get_file_size() > 0);
    CHECK(header.get_original_file_size() > 0);

    // 验证特定文件是否被正确包含
    SECTION("check specific files are included")
    {
        std::set<std::string> expected_files = {
            ".hidden_dir/config",
            ".hidden_file",
            "dir1/test1.txt",
            "dir2/test2.txt",
            "hard_link_file",
            "symlink_file",
            "test1.txt",
            ".hidden_dir",
            "dir1",
            "dir2"
        };

        // 检查每个预期文件是否存在于包中
        for (const auto& p : packets)
        {
            auto ret = expected_files.find(p.info().get_file_name()) != expected_files.end();
            CHECK(ret);
        }
    }

    // 验证链接文件处理
    SECTION("check link handling")
    {
        // 检查硬链接
        bool hard_link_found = false;
        for (const auto& p : packets)
        {
            if (p.info().get_file_name() == "hard_link_file")
            {
                hard_link_found = true;
                // TODO
                break;
            }
        }
        CHECK(hard_link_found == true);

        // 检查符号链接
        bool symlink_found = false;
        for (const auto& p : packets)
        {
            auto name = p.info().get_file_name();
            if (name == "symlink_file")
            {
                symlink_found = true;
                CHECK(p.info().get_file_type() == fs::file_type::symlink);
                // 验证符号链接目标正确
                CHECK(p.info().get_link_name() == "test1.txt");
                break;
            }
        }
        CHECK(symlink_found);
    }

    // 验证隐藏文件处理
    SECTION("check hidden files handling")
    {
        bool hidden_file_found = false;
        bool hidden_dir_found = false;

        for (const auto& p : packets)
        {
            if (p.info().get_file_name() == ".hidden_file")
            {
                hidden_file_found = true;
            }
            else if (p.info().get_file_name() == ".hidden_dir")
            {
                hidden_dir_found = true;
            }
        }

        CHECK(hidden_file_found == true);
        CHECK(hidden_dir_found == true);
    }
}