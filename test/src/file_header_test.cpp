//
// Created by hyh on 2025/10/3.
//

#include"../../include/header/file_header.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("file header","[header]")
{
    using namespace data_packet;

    std::unique_ptr<byte[]> buffer;
    dword checksum = 0;

    SECTION("getter and setter")
    {
        file_header test_header;

        /* version = 2
         * creation_time = 20030820
         * file_number = 12
         * file_size = 123456
         * original_file_size = 234567
         * CRC = 0xffffffff
         */
        test_header.set_version(2);
        REQUIRE(test_header.get_version() == 2);

        test_header.set_file_number(12);
        REQUIRE(test_header.get_file_number() == 12);

        test_header.set_file_size(123456);
        REQUIRE(test_header.get_file_size() == 123456);

        test_header.set_original_file_size(234567);
        REQUIRE(test_header.get_original_file_size() == 234567);

        test_header.refresh_creation_time();
        auto now = std::chrono::system_clock::now();
        auto epoch = now.time_since_epoch();
        auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(epoch).count();

        REQUIRE(timestamp - test_header.get_creation_time() <= 2);


        test_header.set_crc_32(0xffffffff);
        REQUIRE(test_header.get_crc_32() == 0xffffffff);

        test_header.refresh_checksum();
        checksum = test_header.get_checksum();

        buffer = test_header.get_buffer();
    }

    SECTION("buffer")
    {
        file_header test_header;

        /* version = 2
         * creation_time = 20030820
         * file_number = 12
         * file_size = 123456
         * original_file_size = 234567
         * CRC = 0xffffffff
         */

        test_header.set_buffer(buffer.get(),38);
        REQUIRE(test_header.get_version() == 2);
        REQUIRE(test_header.get_file_number() == 12);
        REQUIRE(test_header.get_file_size() == 123456);
        REQUIRE(test_header.get_original_file_size() == 234567);
        REQUIRE(test_header.get_crc_32() == 0xffffffff);
        REQUIRE(test_header.get_checksum() == checksum);
    }
}