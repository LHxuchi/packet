//
// Created by hyh on 2025/10/2.
//

#ifndef DATA_BACK_UP_BYTE_CONVERSION_H
#define DATA_BACK_UP_BYTE_CONVERSION_H

#include <cstdint>
#include <tuple>

namespace data_packet
{
    /// 字
    using word = uint16_t;

    /// 双字
    using dword = uint32_t;

    /// 四字
    using qword = uint64_t;

    /// 字节
    using byte = char;

    /// 双字节
    using two_byte = std::tuple<byte, byte>;

    /// 四字节
    using four_byte = std::tuple<byte, byte, byte, byte>;

    /// 八字节
    using eight_byte = std::tuple<byte, byte,byte, byte,byte, byte,byte, byte>;

    /**
     * @brief 按照大端字节序将字转化为双字节
     * @param wd 指定转化字
     * @return 双字节元组
     */
    two_byte to_bytes(word wd);

    /**
     * @brief 按照大端字节序将双字转化为四字节
     * @param dw 指定转化双字
     * @return 四字节元组
     */
    four_byte to_bytes(dword dw);

    /**
     * @brief 按照大端字节序将四字转化为八字节
     * @param qw 指定转化四字
     * @return 八字节元组
     */
    eight_byte to_bytes(qword qw);

    /**
     * @brief 按照大端字节序将双字节转化为字
     * @param bytes 指定转化双字节元组
     * @return 字
     */
    word make_word(two_byte bytes);

    /**
     * @brief 按照大端字节序将四字节转化为双字
     * @param bytes 指定转化为四字节元组
     * @return 双字
     */
    dword make_dword(four_byte bytes);

    /**
     * @brief 按照大端字节序将八字节转化为四字
     * @param bytes 指定转化八字节元组
     * @return 四字
     */
    qword make_qword(eight_byte bytes);

}

#endif //DATA_BACK_UP_BYTE_CONVERSION_H