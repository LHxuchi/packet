//
// Created by hyh on 2025/10/3.
//

#include "../../include/utils/byte_conversion.h"
#include "../../include/utils/checksum.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("bytes conversion","[utils][byte]")
{
    using namespace data_packet;
    SECTION("bytes to word")
    {
        two_byte bytes;
        std::get<0>(bytes) = (char)0xff;
        std::get<1>(bytes) = (char)0xc1;
        REQUIRE(make_word(bytes) == 0xffc1);
    }

    SECTION("bytes from word")
    {
        word wd = 0xc1f2;
        auto bytes = to_bytes(wd);
        REQUIRE(std::get<0>(bytes) == (char)(0xc1));
        REQUIRE(std::get<1>(bytes) == (char)(0xf2));
    }

    SECTION("bytes to dword")
    {
        four_byte bytes{0xa1,0xb2,0xc3,0xd4};
        dword dw = make_dword(bytes);
        REQUIRE(0xa1b2c3d4 == dw);
    }

    SECTION("bytes from dword")
    {
        dword dw = 0xaabbccdd;
        auto bytes = to_bytes(dw);
        REQUIRE(std::get<0>(bytes) == (char)(0xaa));
        REQUIRE(std::get<1>(bytes) == (char)(0xbb));
        REQUIRE(std::get<2>(bytes) == (char)(0xcc));
        REQUIRE(std::get<3>(bytes) == (char)(0xdd));
    }

    SECTION("bytes to qword")
    {
        eight_byte bytes{0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
        qword qw = make_qword(bytes);
        REQUIRE(0x1122334455667788 == qw);
    }

    SECTION("bytes from qword")
    {
        qword qw = 0x8192a3b4c5d6e7f8;
        auto bytes = to_bytes(qw);
        REQUIRE(std::get<0>(bytes) == (char)(0x81));
        REQUIRE(std::get<1>(bytes) == (char)(0x92));
        REQUIRE(std::get<2>(bytes) == (char)(0xa3));
        REQUIRE(std::get<3>(bytes) == (char)(0xb4));
        REQUIRE(std::get<4>(bytes) == (char)(0xc5));
        REQUIRE(std::get<5>(bytes) == (char)(0xd6));
        REQUIRE(std::get<6>(bytes) == (char)(0xe7));
        REQUIRE(std::get<7>(bytes) == (char)(0xf8));
    }
}

TEST_CASE("checksum","[utils]")
{
    using namespace data_packet;
    auto data1 = std::make_unique<char[]>(4);
    data1[0] = static_cast<char>(1);
    data1[1] = static_cast<char>(2);
    data1[2] = static_cast<char>(3);
    data1[3] = static_cast<char>(4);

    std::vector<std::pair<const char*,size_t>> datas;
    datas.reserve(1);
    datas.emplace_back(data1.get(), 4);

    auto result = calculate_checksum(datas);

    REQUIRE(result == 0xfbfcfdfe);
}