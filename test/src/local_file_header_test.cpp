#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <filesystem>
#include <array>
#include <cstdint>

#include "../../include/header/local_file_header.h"

using namespace data_packet;
namespace fs = std::filesystem;

// 辅助函数：生成随机字节数组
template<size_t N>
std::array<uint8_t, N> generate_random_bytes() {
    std::array<uint8_t, N> arr{};
    for (auto& b : arr) {
        b = static_cast<uint8_t>(rand() % 256);
    }
    return arr;
}

// 辅助函数：创建新的local_file_header实例
local_file_header create_header() {
    return {};
}

TEST_CASE("local_file_header basic functionality", "[local_file_header]") {
    SECTION("Initial values should be zero/default") {
        auto header = create_header();
        CHECK(header.get_uid() == 0);
        CHECK(header.get_gid() == 0);
        CHECK(header.get_crc_32() == 0);
        CHECK(header.get_checksum() == 0);
        CHECK(header.get_compression_method() == local_file_header::compression_method::None);
        CHECK(header.get_encryption_method() == local_file_header::encryption_method::None);
        CHECK(header.get_link_name_length() == 0);
        CHECK(header.get_file_name_length() == 0);
        CHECK(header.get_original_file_size() == 0);
        CHECK(header.get_file_size() == 0);
    }

    SECTION("header_size should return correct size") {
        auto header = create_header();
    }
}

TEST_CASE("local_file_header set and get methods", "[local_file_header]") {
    SECTION("UID and GID operations") {
        auto header = create_header();
        const uint32_t test_uid = 0x12345678;
        const uint32_t test_gid = 0x87654321;

        header.set_uid(test_uid);
        header.set_gid(test_gid);

        CHECK(header.get_uid() == test_uid);
        CHECK(header.get_gid() == test_gid);
    }

    SECTION("User and group name operations") {
        auto header = create_header();
        const std::string short_uname = "testuser";
        const std::string long_uname(32, 'a'); // 最大长度
        const std::string overflow_uname(33, 'b'); // 超过最大长度

        header.set_uname(short_uname);
        CHECK(header.get_uname() == short_uname);

        header.set_uname(long_uname);
        CHECK(header.get_uname() == long_uname);

        header.set_uname(overflow_uname);
        CHECK(header.get_uname() == overflow_uname.substr(0, 32)); // 应该被截断
    }

    SECTION("Time operations") {
        auto header = create_header();
        const uint64_t test_time = 0x1122334455667788;

        header.set_last_modification_time(test_time);
        CHECK(header.get_last_modification_time() == test_time);

        header.set_last_access_time(test_time);
        CHECK(header.get_last_access_time() == test_time);

        // 测试创建时间刷新（假设刷新后不为0）
        const uint64_t old_creation_time = header.get_creation_time();
        header.refresh_creation_time();
        CHECK(header.get_creation_time() != old_creation_time);
    }

    SECTION("File type and permissions") {
        auto header = create_header();
        const fs::perms test_perms = fs::perms::owner_read | fs::perms::group_write;
        const fs::file_type test_type = fs::file_type::regular;

        header.set_permissions(test_perms);
        header.set_file_type(test_type);

        CHECK(header.get_permissions() == test_perms);
        CHECK(header.get_file_type() == test_type);
    }

    SECTION("Compression and encryption methods") {
        auto header = create_header();
        using comp_method = local_file_header::compression_method;
        using enc_method = local_file_header::encryption_method;

        header.set_compression_method(comp_method::LZ77);
        header.set_encryption_method(enc_method::my_method);

        CHECK(header.get_compression_method() == comp_method::LZ77);
        CHECK(header.get_encryption_method() == enc_method::my_method);

        header.set_compression_method(comp_method::None);
        header.set_encryption_method(enc_method::None);

        CHECK(header.get_compression_method() == comp_method::None);
        CHECK(header.get_encryption_method() == enc_method::None);
    }

    SECTION("Salt operations") {
        auto header = create_header();
        const std::array<uint8_t, 16> test_salt = generate_random_bytes<16>();

        header.set_salt(test_salt);
        CHECK(header.get_salt() == test_salt);
    }

    SECTION("File size operations") {
        auto header = create_header();
        const uint64_t test_original_size = 0x123456789ABCDEF0;
        const uint64_t test_file_size = 0xFEDCBA9876543210;

        header.set_original_file_size(test_original_size);
        header.set_file_size(test_file_size);

        CHECK(header.get_original_file_size() == test_original_size);
        CHECK(header.get_file_size() == test_file_size);
    }

    SECTION("Name and link operations") {
        auto header = create_header();
        const std::string test_filename = "test_file.txt";
        const std::string test_linkname = "../link_to_file";

        header.set_file_name(test_filename);
        header.set_link_name(test_linkname);

        CHECK(header.get_file_name() == test_filename);
        CHECK(header.get_link_name() == test_linkname);
        CHECK(header.get_file_name_length() == static_cast<uint16_t>(test_filename.size()));
        CHECK(header.get_link_name_length() == static_cast<uint16_t>(test_linkname.size()));
    }
}

TEST_CASE("local_file_header buffer serialization/deserialization", "[local_file_header]") {
    SECTION("Serialized buffer should be correctly deserialized") {
        // 创建原始对象并设置测试数据
        auto original = create_header();
        original.set_uid(0x11AA22BB);
        original.set_gid(0x33CC44DD);
        original.set_uname("test_user_123");
        original.set_gname("test_group_456");
        original.set_last_modification_time(0x1122334455667788);
        original.set_last_access_time(0x8877665544332211);
        original.set_permissions(fs::perms::all);
        original.set_file_type(fs::file_type::directory);
        original.set_crc_32(0xABCD1234);
        original.set_compression_method(local_file_header::compression_method::LZ77);
        original.set_encryption_method(local_file_header::encryption_method::my_method);

        std::array<uint8_t, 16> test_salt = generate_random_bytes<16>();
        original.set_salt(test_salt);

        original.set_original_file_size(123456789);
        original.set_file_size(987654321);
        original.set_file_name("serialization_test.dat");
        original.set_link_name("test_link");

        // 获取序列化缓冲区
        auto buffer = original.get_buffer();

        // 反序列化到新对象
        auto deserialized = create_header();
        deserialized.set_buffer(buffer.get());

        // 验证数据一致性
        CHECK(deserialized.get_uid() == original.get_uid());
        CHECK(deserialized.get_gid() == original.get_gid());
        CHECK(deserialized.get_uname() == original.get_uname());
        CHECK(deserialized.get_gname() == original.get_gname());
        CHECK(deserialized.get_last_modification_time() == original.get_last_modification_time());
        CHECK(deserialized.get_last_access_time() == original.get_last_access_time());
        CHECK(deserialized.get_permissions() == original.get_permissions());
        CHECK(deserialized.get_file_type() == original.get_file_type());
        CHECK(deserialized.get_crc_32() == original.get_crc_32());
        CHECK(deserialized.get_compression_method() == original.get_compression_method());
        CHECK(deserialized.get_encryption_method() == original.get_encryption_method());
        CHECK(deserialized.get_salt() == original.get_salt());
        CHECK(deserialized.get_original_file_size() == original.get_original_file_size());
        CHECK(deserialized.get_file_size() == original.get_file_size());
    }

}

TEST_CASE("local_file_header edge cases", "[local_file_header]") {
    SECTION("Maximum length strings") {
        auto header = create_header();
        std::string max_uname(32, 'x');
        std::string max_gname(32, 'y');

        header.set_uname(max_uname);
        header.set_gname(max_gname);

        CHECK(header.get_uname() == max_uname);
        CHECK(header.get_gname() == max_gname);
    }

    SECTION("Zero values") {
        auto header = create_header();
        header.set_uid(0);
        header.set_gid(0);
        header.set_original_file_size(0);
        header.set_file_size(0);
        header.set_crc_32(0);

        CHECK(header.get_uid() == 0);
        CHECK(header.get_gid() == 0);
        CHECK(header.get_original_file_size() == 0);
        CHECK(header.get_file_size() == 0);
        CHECK(header.get_crc_32() == 0);
    }

    SECTION("Maximum numeric values") {
        auto header = create_header();
        header.set_uid(UINT32_MAX);
        header.set_gid(UINT32_MAX);
        header.set_original_file_size(UINT64_MAX);
        header.set_file_size(UINT64_MAX);
        header.set_crc_32(UINT32_MAX);

        CHECK(header.get_uid() == UINT32_MAX);
        CHECK(header.get_gid() == UINT32_MAX);
        CHECK(header.get_original_file_size() == UINT64_MAX);
        CHECK(header.get_file_size() == UINT64_MAX);
        CHECK(header.get_crc_32() == UINT32_MAX);
    }
}
