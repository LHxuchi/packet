#include <catch2/catch_test_macros.hpp>
#include <vector>
#include <cstring>
#include "../../include/encryption_method/encryption.h"
#include <fstream>


// 测试数据分组加密解密接口
TEST_CASE("Data Packet Encryption/Decryption", "[data_packet]") {
    const std::string password = "PacketTestPassword";
    std::vector<data_packet::byte> test_data = {'a', 'b', 'c', static_cast<data_packet::byte>(0x00), static_cast<data_packet::byte>(0xFF), 'x', 'y', 'z'};

    SECTION("迭代器范围加密解密") {
        auto encrypted = data_packet::encrypt(test_data.begin(), test_data.end(), password);
        REQUIRE(encrypted.second > 0);
        REQUIRE(encrypted.first != nullptr);

        std::vector<data_packet::byte> cipher_vec(encrypted.first.get(), encrypted.first.get() + encrypted.second);
        auto decrypted = data_packet::decrypt(cipher_vec.begin(), cipher_vec.end(), password);
        REQUIRE(decrypted.second == test_data.size());
        REQUIRE(std::memcmp(decrypted.first.get(), test_data.data(), test_data.size()) == 0);
    }

    SECTION("缓冲区大小加密解密") {
        auto encrypted = data_packet::encrypt(test_data.data(), test_data.size(), password);
        REQUIRE(encrypted.second > 0);

        auto decrypted = data_packet::decrypt(encrypted.first.get(), encrypted.second, password);
        REQUIRE(decrypted.second == test_data.size());
        REQUIRE(std::memcmp(decrypted.first.get(), test_data.data(), test_data.size()) == 0);
    }

    SECTION("空数据处理") {
        std::vector<data_packet::byte> empty_data;
        auto encrypted = data_packet::encrypt(empty_data.begin(), empty_data.end(), password);
        REQUIRE(encrypted.second > 0);

        auto decrypted = data_packet::decrypt(encrypted.first.get(), encrypted.second, password);
        REQUIRE(decrypted.second == 0);
    }

    SECTION("错误密码解密失败") {
        auto encrypted = data_packet::encrypt(test_data.begin(), test_data.end(), password);
        auto decrypted = data_packet::decrypt(encrypted.first.get(), encrypted.second, "wrong_pass");
        REQUIRE(decrypted.first == nullptr);
        REQUIRE(decrypted.second == 0);
        std::cout<<"wrong password decrypt test passed."<< std::endl;

    }
    SECTION("文件加密解密测试")
    {
        std::ifstream file(R"(../test/static/test.txt)");
        if(!file.is_open())
        {
            std::cerr << "Failed to open file " << R"(../test/static/test.txt)" << std::endl;
        }
        std::stringstream buffer;

        buffer << file.rdbuf();

        std::string content = buffer.str();

        auto result = data_packet::encrypt(content.begin(),content.end());
        auto str = data_packet::decrypt(result.first.get(),result.first.get() + result.second);
        
        REQUIRE(str.second == content.size());

        bool compare_result = std::equal(content.begin(),content.end(),str.first.get());

        REQUIRE(compare_result);

        std::cout<<"encrypt/decrypt is tested."<< std::endl;

    }
}