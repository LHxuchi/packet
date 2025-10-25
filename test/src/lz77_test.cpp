//
// Created by hyh on 2025/10/21.
//

#include "../../include/compression_method/lz77.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("compression position test","[lz77][compression]")
{
    using namespace data_packet;

    std::string content{"abcabcdaaabcdddabc"};
    std::string pattern{"abcd"};

    auto info = detail::compression_tuple
    ({content.begin(),content.end()},{pattern.begin(),pattern.end()},'x');

    CHECK(std::get<0>(info) == 9);
    CHECK(std::get<1>(info) == 4);
    CHECK(std::get<2>(info) == 'x');

    pattern.pop_back();
    info = detail::compression_tuple
    ({content.begin(),content.end()},{pattern.begin(),pattern.end()},'x');

    CHECK(std::get<0>(info) == 3);
    CHECK(std::get<1>(info) == 3);
    CHECK(std::get<2>(info) == 'x');

    pattern.pop_back();
    info = detail::compression_tuple
    ({content.begin(),content.end()},{pattern.begin(),pattern.end()},'x');

    CHECK(std::get<0>(info) == 3);
    CHECK(std::get<1>(info) == 2);
    CHECK(std::get<2>(info) == 'x');

    pattern = "abcde";
    info = detail::compression_tuple
    ({content.begin(),content.end()},{pattern.begin(),pattern.end()},'x');
    CHECK(std::get<0>(info) == 9);
    CHECK(std::get<1>(info) == 4);
    CHECK(std::get<2>(info) == 'e');

    content = "aaaaa";
    pattern = "aaaaaaaaaaaa";
    info = detail::compression_tuple
    ({content.begin(),content.end()},{pattern.begin(),pattern.end()},'x');
    CHECK(std::get<0>(info) == 5);
    CHECK(std::get<1>(info) == 5);
    CHECK(std::get<2>(info) == 'a');

    SECTION("compress short content")
    {
        content = "abcabcaaaabccccbcd";
        auto result = lz77_compress(content.begin(),content.end());

        auto str = lz77_decompress(result.first.get(),result.first.get() + result.second);

        REQUIRE(str.second == content.size());

        CHECK(std::string(str.first.get(),str.second) == content);

        content = "A Comprehensive Exploration of Digital Technology and Its Societal Impacts";
        result = lz77_compress(content.begin(),content.end());
        str = lz77_decompress(result.first.get(),result.first.get() + result.second);

        REQUIRE(str.second == content.size());

        CHECK(std::string(str.first.get(),str.second) == content);
    }

    SECTION("compress long content")
    {
        std::ifstream file(R"(../test/static/test.txt)");
        if(!file.is_open())
        {
            std::cerr << "Failed to open file " << R"(../test/static/test.txt)" << std::endl;
        }
        std::stringstream buffer;

        buffer << file.rdbuf();

        content = buffer.str();

        auto result = lz77_compress(content.begin(),content.end());
        auto str = lz77_decompress(result.first.get(),result.first.get() + result.second);

        REQUIRE(str.second == content.size());

        bool compare_result = std::equal(content.begin(),content.end(),str.first.get());

        REQUIRE(compare_result);

        std::cout<<"Compressing rate is "<< static_cast<double>(str.second) / static_cast<double>(result.second) <<std::endl;

    }
}