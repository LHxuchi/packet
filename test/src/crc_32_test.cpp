//
// Created by hyh on 2025/10/8.
//

#include "../../include/utils/crc_32.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("calculate and verify crc_32 code","[crc_32][utils]")
{
    // 计算得到crc为0xC07A9F32
    std::string str{"This is a test"};

    auto result = data_packet::CRC_calculate(str.begin(), str.end());

    REQUIRE(result == 0xc07a9f32);
    REQUIRE(data_packet::CRC_verify(result, str.begin(), str.end()));

    result += 1;
    REQUIRE(!data_packet::CRC_verify(result, str.begin(), str.end()));
}