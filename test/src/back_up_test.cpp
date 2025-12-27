// 引入Catch2框架头文件（v3版本，若使用v2请改为 <catch.hpp>）
#include <catch2/catch_test_macros.hpp>
// 引入待测试的头文件
#include "../../include/back_up/back_up.h"
// 辅助头文件
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

// 命名空间别名简化代码
namespace fs = std::filesystem;
namespace dp = data_packet;

// 测试套件：数据备份核心功能测试（修正destination为文件路径 + 前置清理）
TEST_CASE("DataBackup Core Functionality Tests (Destination as File Path + Pre-Cleanup)", "[backup][restore][info]") {
    // ========== 新增：测试前置清理逻辑（核心修改点） ==========
    // 定义唯一临时目录路径（与后续测试目录一致）
    fs::path temp_root = fs::temp_directory_path() / "data_backup_test" ;
    // 前置清理：检查如果临时目录已存在，先递归删除所有内容
    if (fs::exists(temp_root)) {
        REQUIRE_NOTHROW(fs::remove_all(temp_root)); // 强制删除目录及所有子文件/子目录
        REQUIRE_FALSE(fs::exists(temp_root)); // 验证前置清理成功
    }

    // ========== 测试前置准备：创建临时目录和测试文件 ==========
    fs::path test_source_dir = temp_root / "source_dir";
    fs::path test_dest_dir = temp_root / "dest_dir"; // 备份文件所在目录（非backup函数的destination参数）
    fs::path backup_file_path = test_dest_dir / "test_backup.backup"; // back_up函数的destination：明确文件路径
    fs::path exclude_file1 = test_source_dir / "exclude_1.txt";
    fs::path include_file1 = test_source_dir / "include_1.txt";
    fs::path include_subdir = test_source_dir / "subdir";
    fs::path include_file2 = include_subdir / "include_2.txt";

    // 确保目录存在（仅创建目录，备份文件由back_up函数自动生成）
    REQUIRE_NOTHROW(fs::create_directories(test_source_dir));
    REQUIRE_NOTHROW(fs::create_directories(test_dest_dir)); // 确保备份文件所在目录存在
    REQUIRE_NOTHROW(fs::create_directories(include_subdir));

    // 创建测试文件并写入内容
    std::ofstream(exclude_file1) << "This file should be excluded from backup.";
    std::ofstream(include_file1) << "This file should be included in backup.";
    std::ofstream(include_file2) << "This subdir file should be included in backup.";

    // 验证测试文件创建成功
    REQUIRE(fs::exists(exclude_file1));
    REQUIRE(fs::exists(include_file1));
    REQUIRE(fs::exists(include_file2));

    // 初始状态：备份文件不存在
    REQUIRE_FALSE(fs::exists(backup_file_path));

    // ========== Test Case 1: 正常备份（无压缩、无加密，指定文件路径作为destination） ==========
    SECTION("Normal Backup (NONE compression, NONE encryption, destination as file path)") {
        // 排除文件列表（相对路径，换行分割）
        std::string exclude_files = "exclude_1.txt\nsubdir/nonexist_file.txt"; // 包含不存在的排除文件，验证兼容性
        // 执行备份：destination传入明确的文件路径（backup_file_path）
        std::string backup_result = dp::back_up(
            test_source_dir,
            backup_file_path, // 修正：传入文件路径而非目录路径
            "NONE",
            "NONE",
            "", // 无加密，密码为空
            exclude_files
        );

        // 验证备份执行成功
        REQUIRE(backup_result == "OK");
        // 直接验证指定路径的备份文件生成（无需遍历目录）
        REQUIRE(fs::exists(backup_file_path));
        REQUIRE(fs::is_regular_file(backup_file_path)); // 验证是普通文件

        // ========== Test Case 2: 获取备份包信息 ==========
        SECTION("Get Backup Info") {
            std::string info_result = dp::info(backup_file_path);
            // 验证信息获取无错误（非空，且符合key: value格式）
            REQUIRE_FALSE(info_result.empty());
            REQUIRE(info_result.find(':') != std::string::npos); // 验证键值对格式
        }

        // ========== Test Case 3: 正常还原备份（无加密） ==========
        SECTION("Normal Restore Backup (NONE encryption)") {
            // 创建还原目标目录
            fs::path restore_dir = temp_root / "restore_dir";
            REQUIRE_NOTHROW(fs::create_directories(restore_dir));

            // 执行还原
            std::string restore_result = dp::restore_backup(
                backup_file_path, // 备份包文件路径
                restore_dir,
                "" // 无加密，密码为空
            );

            // 验证还原执行成功
            REQUIRE(restore_result == "OK");

            // 验证还原后的文件存在（排除的文件不应存在，包含的文件应存在）
            fs::path restored_include1 = restore_dir / "include_1.txt";
            fs::path restored_include2 = restore_dir / "subdir" / "include_2.txt";
            fs::path restored_exclude1 = restore_dir / "exclude_1.txt";

            REQUIRE(fs::exists(restored_include1));
            REQUIRE(fs::exists(restored_include2));
            REQUIRE_FALSE(fs::exists(restored_exclude1)); // 验证排除文件未被还原

            // 验证还原文件内容一致（增强测试严谨性）
            std::string original_content1, restored_content1;
            std::ifstream ifs_original(include_file1);
            std::ifstream ifs_restored(restored_include1);
            std::getline(ifs_original, original_content1);
            std::getline(ifs_restored, restored_content1);
            REQUIRE(original_content1 == restored_content1);
        }
    }

    // ========== Test Case 4: 异常场景 - 源目录不存在 ==========
    SECTION("Exception: Source Directory Not Exists") {
        fs::path nonexist_source = temp_root / "nonexist_source_dir";
        // 备份文件路径仍指定有效目录下的文件
        fs::path temp_backup_file = test_dest_dir / "nonexist_source_backup.backup";

        std::string backup_result = dp::back_up(
            nonexist_source,
            temp_backup_file, // 目标文件路径
            "NONE",
            "NONE",
            "",
            ""
        );

        // 验证返回非OK（报错信息）
        REQUIRE(backup_result != "OK");
        // 源目录不存在时，备份文件不应生成
        REQUIRE_FALSE(fs::exists(temp_backup_file));
    }

    // ========== Test Case 5: 异常场景 - 备份（LZ77压缩 + AES_256_CBC加密，指定文件路径） ==========
    SECTION("Backup with LZ77 Compression and AES_256_CBC Encryption") {
        // 指定加密备份的文件路径
        fs::path encrypted_backup_file = test_dest_dir / "encrypted_test_backup.backup";
        std::string exclude_files = "exclude_1.txt";
        std::string password = "Test@123456"; // 测试密码

        // 执行加密压缩备份
        std::string backup_result = dp::back_up(
            test_source_dir,
            encrypted_backup_file, // 修正：传入加密备份的文件路径
            "LZ77",
            "AES_256_CBC",
            password,
            exclude_files
        );

        REQUIRE(backup_result == "OK");
        REQUIRE(fs::exists(encrypted_backup_file)); // 验证加密备份文件生成

        // 还原加密备份（正确密码）
        fs::path encrypted_restore_dir = temp_root / "encrypted_restore_dir";
        REQUIRE_NOTHROW(fs::create_directories(encrypted_restore_dir));
        std::string restore_result = dp::restore_backup(
            encrypted_backup_file,
            encrypted_restore_dir,
            password
        );
        REQUIRE(restore_result == "OK");

        // 验证错误密码还原失败
        std::string wrong_password = "Wrong@123456";
        fs::path wrong_pwd_restore_dir = temp_root / "wrong_pwd_restore_dir";
        fs::create_directories(wrong_pwd_restore_dir);
        std::string wrong_pwd_restore_result = dp::restore_backup(
            encrypted_backup_file,
            wrong_pwd_restore_dir,
            wrong_password
        );
        REQUIRE(wrong_pwd_restore_result != "OK");
    }

}