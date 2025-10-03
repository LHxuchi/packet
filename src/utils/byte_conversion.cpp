//
// Created by hyh on 2025/10/2.
//

#include "../../include/utils/byte_conversion.h"

data_packet::two_byte data_packet::to_bytes(word wd)
{
    two_byte ret;
    std::get<1>(ret) = static_cast<byte>(wd & 0xff);
    std::get<0>(ret) = static_cast<byte>((wd >> 8) & 0xff);
    return ret;
}

data_packet::four_byte data_packet::to_bytes(dword dw)
{
    four_byte ret;
    std::get<3>(ret) = static_cast<byte>(dw & 0xff);
    std::get<2>(ret) = static_cast<byte>((dw >> 8) & 0xff);
    std::get<1>(ret) = static_cast<byte>((dw >> 16) & 0xff);
    std::get<0>(ret) = static_cast<byte>((dw >> 24) & 0xff);
    return ret;
}

data_packet::eight_byte data_packet::to_bytes(qword qw)
{
    eight_byte ret;
    std::get<7>(ret) = static_cast<byte>(qw & 0xff);
    std::get<6>(ret) = static_cast<byte>((qw >> 8) & 0xff);
    std::get<5>(ret) = static_cast<byte>((qw >> 16) & 0xff);
    std::get<4>(ret) = static_cast<byte>((qw >> 24) & 0xff);
    std::get<3>(ret) = static_cast<byte>((qw >> 32) & 0xff);
    std::get<2>(ret) = static_cast<byte>((qw >> 40) & 0xff);
    std::get<1>(ret) = static_cast<byte>((qw >> 48) & 0xff);
    std::get<0>(ret) = static_cast<byte>((qw >> 56) & 0xff);
    return ret;
}

data_packet::word data_packet::make_word(two_byte bytes)
{
    return static_cast<word>(
        (static_cast<word>(std::get<0>(bytes)) << 8) |  // 高8位字节
        (static_cast<word>(std::get<1>(bytes)) & 0xFF)  // 低8位字节
    );
}

data_packet::dword data_packet::make_dword(four_byte bytes)
{
    return static_cast<dword>(
        ((static_cast<dword>(std::get<0>(bytes)) << 24) & 0xFF000000) |
        ((static_cast<dword>(std::get<1>(bytes)) << 16) & 0xFF0000) |
        ((static_cast<dword>(std::get<2>(bytes)) << 8)& 0xFF00) |
        (static_cast<dword>(std::get<3>(bytes)) & 0xFF)
    );
}

data_packet::qword data_packet::make_qword(eight_byte bytes)
{
    return static_cast<qword>(
        (static_cast<qword>(std::get<0>(bytes)) << 56) & 0xFF00000000000000ULL |
        (static_cast<qword>(std::get<1>(bytes)) << 48) & 0xFF000000000000ULL |
        (static_cast<qword>(std::get<2>(bytes)) << 40) & 0xFF0000000000ULL |
        (static_cast<qword>(std::get<3>(bytes)) << 32) & 0xFF00000000ULL |
        (static_cast<qword>(std::get<4>(bytes)) << 24) & 0xFF000000ULL |
        (static_cast<qword>(std::get<5>(bytes)) << 16) & 0xFF0000ULL |
        (static_cast<qword>(std::get<6>(bytes)) << 8)  & 0xFF00ULL |
        (static_cast<qword>(std::get<7>(bytes)) & 0xFF)
    );
}
