//
// Created by hyh on 2025/10/6.
//

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_session.hpp>
#include <filesystem>
#include <fstream>
#include <set>
#include "../../include/file_system/get_entries.h"  // 包含get_entries函数的头文件

namespace fs = std::filesystem;
using namespace data_packet;

// 创建测试用的临时目录结构（Linux专用）
fs::path create_linux_test_directory() {
    // 创建唯一临时目录
    auto temp_dir = fs::temp_directory_path() / "linux_entries_test_";
    fs::create_directories(temp_dir);
    fs::permissions(temp_dir, fs::perms::all);  // 确保有完整权限

    // 创建普通文件（使用std::ofstream，需要包含<fstream>）
    {
        std::ofstream file(temp_dir / "normal_file1.txt");
        file.put('a'); // 写入一个字符，确保文件存在
    }
    {
        std::ofstream file(temp_dir / "normal_file2.dat");
        file.put('b');
    }

    // 创建Linux隐藏文件（以.开头）
    {
        std::ofstream file(temp_dir / ".hidden_file");
        file.put('c');
    }
    {
        std::ofstream file(temp_dir / ".config");
        file.put('d');
    }

    // 创建子目录
    auto subdir = temp_dir / "subdirectory";
    fs::create_directory(subdir);
    {
        std::ofstream file(subdir / "subfile.txt");
        file.put('e');
    }

    return temp_dir;
}

// 清理测试目录
void cleanup_test_directory(const fs::path& dir) {
    if (fs::exists(dir)) {
        fs::remove_all(dir);
    }
}

TEST_CASE("Linux get_entries tests", "[data_packet][file_system]") {
    auto test_dir = create_linux_test_directory();
    auto cleanup = [&](){ cleanup_test_directory(test_dir); };

    SECTION("Retrieve all files including hidden ones") {
        auto entries = get_entries(test_dir);

        // 验证总数：3个普通文件 + 2个隐藏文件 + 1个目录
        REQUIRE(entries.size() == 6);

        // 验证普通文件存在
        CHECK(entries.count(fs::directory_entry(test_dir / "normal_file1.txt")) == 1);
        CHECK(entries.count(fs::directory_entry(test_dir / "normal_file2.dat")) == 1);

        // 验证隐藏文件存在
        CHECK(entries.count(fs::directory_entry(test_dir / ".hidden_file")) == 1);
        CHECK(entries.count(fs::directory_entry(test_dir / ".config")) == 1);

        // 验证子目录
        CHECK(entries.count(fs::directory_entry(test_dir / "subdirectory")) == 1);
        CHECK(entries.count(fs::directory_entry(test_dir / "subdirectory" / "subfile.txt")) == 1);
    }

    SECTION("Handle non-existent path") {
        auto non_existent = test_dir / "this_dir_does_not_exist";
        auto entries = get_entries(non_existent);
        CHECK(entries.empty());
    }

    SECTION("Handle empty directory") {
        auto empty_dir = test_dir / "empty_dir";
        fs::create_directory(empty_dir);
        auto entries = get_entries(empty_dir);
        CHECK(entries.empty());
    }

    SECTION("Handle symlinks (Linux specific)") {
        auto link_path = test_dir / "link_to_file";
        fs::create_symlink(test_dir / "normal_file1.txt", link_path);

        auto entries = get_entries(test_dir);
        CHECK(entries.count(fs::directory_entry(link_path)) == 1);
    }

    SECTION("Filter test")
    {
        auto entries = get_entries(test_dir,
            [](const std::filesystem::directory_entry& entry){return false;});
        // 验证总数：3个普通文件 + 2个隐藏文件 + 1个目录
        REQUIRE(entries.size() == 6);

        // 验证普通文件存在
        CHECK(entries.count(fs::directory_entry(test_dir / "normal_file1.txt")) == 1);
        CHECK(entries.count(fs::directory_entry(test_dir / "normal_file2.dat")) == 1);

        // 验证隐藏文件存在
        CHECK(entries.count(fs::directory_entry(test_dir / ".hidden_file")) == 1);
        CHECK(entries.count(fs::directory_entry(test_dir / ".config")) == 1);

        // 验证子目录
        CHECK(entries.count(fs::directory_entry(test_dir / "subdirectory")) == 1);
        CHECK(entries.count(fs::directory_entry(test_dir / "subdirectory" / "subfile.txt")) == 1);

        entries = get_entries(test_dir,
            [](const std::filesystem::directory_entry& entry)
            {return entry.path().string().find(".txt") != std::string::npos; });

        // 验证普通文件部分存在
        CHECK(entries.count(fs::directory_entry(test_dir / "normal_file1.txt")) == 0);
        CHECK(entries.count(fs::directory_entry(test_dir / "normal_file2.dat")) == 1);

        // 验证隐藏文件存在
        CHECK(entries.count(fs::directory_entry(test_dir / ".hidden_file")) == 1);
        CHECK(entries.count(fs::directory_entry(test_dir / ".config")) == 1);

        // 验证子目录，文件部分存在
        CHECK(entries.count(fs::directory_entry(test_dir / "subdirectory")) == 1);
        CHECK(entries.count(fs::directory_entry(test_dir / "subdirectory" / "subfile.txt")) == 0);

    }

    cleanup();
}