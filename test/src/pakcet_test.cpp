//
// Created by hyh on 2025/10/10.
//

#include <fstream>
#include <iostream>
#include <set>
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_session.hpp>

#include "../../include/packet/packet.h"
#include "file_system/get_entries.h"

TEST_CASE("verify packet content and structure", "[packet]")
{
    namespace dp = data_packet;
    namespace fs = std::filesystem;

    // 创建目录结构
    system("mkdir mytest");
    system("mkdir mytest/dir1");
    system("mkdir mytest/dir2");
    system("mkdir mytest/.hidden_dir");


    // 创建文件
    system("touch mytest/test1.txt");
    system("touch mytest/.hidden_file");
    system("touch mytest/dir1/test1.txt");
    system("touch mytest/dir2/test2.txt");
    system("touch mytest/.hidden_dir/config");
    system("touch mytest/test_long_file.txt");

    // 创建链接
    system("ln -s test1.txt mytest/symlink_file");
    system("ln mytest/test1.txt mytest/hard_link_file");

    auto test_path = fs::path("./mytest");
    test_path = fs::canonical(test_path);

    // 定义每个文件对应的写入内容（键为相对路径，值为内容）
    std::map<std::string, std::string> file_contents = {
        {"test1.txt", "Content for root test file"},
        {".hidden_file", "Content for hidden file"},
        {"dir1/test1.txt", "Content for dir1 test file"},
        {"dir2/test2.txt", "Content for dir2 test file"},
        {".hidden_dir/config", "Content for hidden dir config file"}
    };

    std::ifstream ifs(R"(/home/hyh/CLionProjects/packet/test/static/test.txt)");
    std::string long_content{std::istreambuf_iterator<char>(ifs),std::istreambuf_iterator<char>()};
    file_contents.insert({"test_long_file.txt", long_content});


    // 遍历所有需要写入内容的文件
    for (const auto& [rel_path, content] : file_contents) {
        // 构建完整路径
        fs::path full_path = test_path / rel_path;

        // 检查文件是否存在且为普通文件
        if (fs::exists(full_path) && fs::is_regular_file(full_path)) {
            // 尝试打开文件并写入内容
            std::ofstream ofs(full_path);
            CHECK(ofs.is_open());

            ofs << content;
            ofs.close();

        } else {
            FAIL("File not found or not a regular file: " << full_path.string());
        }
    }

    // 打包
    auto pkt = dp::make_packet(test_path);

    // 验证文件总数（与现有测试呼应，确认目录结构解析正确）
    CHECK(pkt.info().get_file_number() == 11);

    // 验证基本头信息有效性
    CHECK(pkt.info().get_version() != 0);  // 假设版本号已正确设置
    CHECK(pkt.info().get_creation_time() != 0);  // 验证创建时间已刷新
    CHECK(pkt.info().get_checksum() != 0);  // 验证校验和已计算
    CHECK(pkt.info().get_crc_32() != 0);    // 验证CRC32已计算

    // 验证总文件大小和原始文件大小为正数
    CHECK(pkt.info().get_file_size() >= 0);
    CHECK(pkt.info().get_original_file_size() >= 0);




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
            "dir2",
            "test_long_file.txt"
        };

        // 检查每个预期文件是否存在于包中
        unsigned int count = 0;
        for (const auto& p : pkt.packets())
        {
            auto ret = expected_files.find(p.info().get_file_name()) != expected_files.end();
            CHECK(ret);
            ++count;
        }
        CHECK(count == 11);
    }

    SECTION("check directory and fifo")
    {
        unsigned int count = 0;
        for (const auto& p : pkt.packets())
        {

            if (p.info().get_file_type() == fs::file_type::directory || p.info().get_file_type() == fs::file_type::fifo)
            {
                CHECK(p.info().get_file_size() == 0);
                CHECK(p.info().get_original_file_size() == 0);
                CHECK(p.info().get_link_name_length() == 0);
                count++;
            }

        }
        CHECK(count == 3);
    }

    // 验证链接文件处理
    SECTION("check link handling")
    {
        // 检查硬链接
        bool hard_link_found = false;
        for (const auto& p : pkt.packets())
        {
            if (p.info().get_file_name() == "hard_link_file" && p.info().get_link_name_length() > 0)
            {
                hard_link_found = true;
                CHECK(p.info().get_original_file_size() == 11);
                CHECK(p.info().get_file_size() == 11);
                CHECK(p.info().get_link_name_length() > 0);
                CHECK(p.info().get_link_name() == "test1.txt");
                break;
            }
            if (p.info().get_file_name() == "test1.txt" && p.info().get_link_name_length() > 0)
            {
                hard_link_found = true;
                CHECK(p.info().get_original_file_size() == 11);
                CHECK(p.info().get_file_size() == 11);
                CHECK(p.info().get_link_name_length() > 0);
                CHECK(p.info().get_link_name() == "hard_link_file");
                break;
            }
        }
        CHECK(hard_link_found == true);

        // 检查符号链接
        bool symlink_found = false;
        for (const auto& p : pkt.packets())
        {
            auto name = p.info().get_file_name();
            if (name == "symlink_file")
            {
                symlink_found = true;
                CHECK(p.info().get_file_type() == fs::file_type::symlink);
                // 验证符号链接目标正确
                CHECK(p.info().get_link_name() == "/home/hyh/CLionProjects/packet/bin/mytest/test1.txt");
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

        for (const auto& p : pkt.packets())
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

    // 测试文件其他参数是否设置正确
    SECTION("file size and original size")
    {
        for (const auto& p : pkt.packets())
        {
            CHECK(p.info().get_original_file_size() == p.info().get_file_size());
        }
    }


    SECTION("write and read")
    {
        // 写入数据包到文件
        std::fstream my_file("/home/hyh/CLionProjects/packet/bin/packet.pkt",
            std::ios::in | std::ios::out | std::ios::binary | std::ios::app);

        if (!my_file.is_open())
        {
            std::cerr << "Failed to open file" << std::endl;
            throw std::runtime_error("Failed to open file");
        }

        my_file.seekg(0, std::ios::beg);
        my_file << pkt;

        my_file.seekg(0,std::ios::beg);  // 关键：重置读指针到开头

        // 从文件读取数据包
        decltype(pkt) in_pkt;
        my_file >> in_pkt;

        my_file.close();

        // 清理测试文件
        fs::remove("packet.pkt");

        // 验证头信息是否一致
        CHECK(in_pkt.info().get_file_number() == pkt.info().get_file_number());
        CHECK(in_pkt.info().get_version() == pkt.info().get_version());
        CHECK(in_pkt.info().get_file_size() == pkt.info().get_file_size());
        CHECK(in_pkt.info().get_original_file_size() == pkt.info().get_original_file_size());
        CHECK(in_pkt.info().get_checksum() == pkt.info().get_checksum());
        CHECK(in_pkt.info().get_crc_32() == pkt.info().get_crc_32());
        CHECK(in_pkt.info().get_creation_time() == pkt.info().get_creation_time());

        // 验证数据包数量一致
        CHECK(in_pkt.packets().size() == pkt.packets().size());

        // 验证每个文件的信息一致
        for (unsigned int i = 0; i < in_pkt.packets().size(); i++)
        {
            CHECK(in_pkt.packets()[i].info().get_original_file_size() == pkt.packets()[i].info().get_original_file_size());
            CHECK(in_pkt.packets()[i].info().get_file_size() == pkt.packets()[i].info().get_file_size());

            // 补充用户ID和组ID验证
            CHECK(in_pkt.packets()[i].info().get_uid() == pkt.packets()[i].info().get_uid());
            CHECK(in_pkt.packets()[i].info().get_gid() == pkt.packets()[i].info().get_gid());

            // 用户名和组名验证
            CHECK(std::string(in_pkt.packets()[i].info().get_uname()) == std::string(pkt.packets()[i].info().get_uname()));
            CHECK(std::string(in_pkt.packets()[i].info().get_gname()) == std::string(pkt.packets()[i].info().get_gname()));

            // 时间戳验证
            CHECK(in_pkt.packets()[i].info().get_creation_time() == pkt.packets()[i].info().get_creation_time());
            CHECK(in_pkt.packets()[i].info().get_last_modification_time() == pkt.packets()[i].info().get_last_modification_time());
            CHECK(in_pkt.packets()[i].info().get_last_access_time() == pkt.packets()[i].info().get_last_access_time());

            // 加密盐值验证（假设返回字节数组或其字符串表示）
            bool result = false;
            auto arr1 = in_pkt.packets()[i].info().get_salt();
            auto arr2 = pkt.packets()[i].info().get_salt();
            result = std::equal(arr1.begin(), arr1.end(), arr2.begin());
            CHECK(result);

            // 链接名和文件名长度验证
            CHECK(in_pkt.packets()[i].info().get_link_name_length() == pkt.packets()[i].info().get_link_name_length());
            CHECK(in_pkt.packets()[i].info().get_file_name_length() == pkt.packets()[i].info().get_file_name_length());

            // 校验值检验
            CHECK(in_pkt.packets()[i].info().get_checksum() == pkt.packets()[i].info().get_checksum());
            CHECK(in_pkt.packets()[i].info().get_crc_32() == pkt.packets()[i].info().get_crc_32());

            // 权限校验
            CHECK(in_pkt.packets()[i].info().get_permissions() == pkt.packets()[i].info().get_permissions());
        }
    }

    SECTION("unpack")
    {
        fs::path unpack_path = R"(/home/hyh/test)";
        fs::create_directory(unpack_path);
        dp::unpack_packet(unpack_path,pkt);

        for (const auto& [name,content] : file_contents)
        {
            std::ifstream file(unpack_path/name);
            std::string input_string{std::istreambuf_iterator<char>(file),std::istreambuf_iterator<char>()};
            CHECK(input_string == content);
        }

        fs::remove_all(unpack_path);
    }

    system("rm -rf mytest");

}