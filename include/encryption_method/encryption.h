#include <iostream>
#include <string>
#include <cstring>
#include "../utils/byte_conversion.h"
#include <memory>
#include <utility>
#include <iterator>

#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <openssl/err.h>
namespace encryption{
// 兼容定义（若未定义）
#ifndef AES_BLOCK_SIZE
#define AES_BLOCK_SIZE 16
#endif
#ifndef AES_KEY_SIZE_256
#define AES_KEY_SIZE_256 32
#endif

// 错误处理宏（兼容OpenSSL 3.x）
#define CHECK_OPENSSL_ERR(func) \
    do { \
        unsigned long err = ERR_get_error(); \
        if (err != 0) { \
            char err_buf[256] = {0}; \
            ERR_error_string_n(err, err_buf, sizeof(err_buf)); \
            std::cerr << "OpenSSL error in " #func ": " << err_buf << std::endl; \
            return false; \
        } \
    } while (0)

// PKCS7填充（补全到block_size的整数倍）
std::string pkcs7_pad(const std::string& data, size_t block_size) {
    size_t pad_len = block_size - (data.size() % block_size);
    std::string padded = data;
    padded.append(pad_len, static_cast<char>(pad_len));
    return padded;
}

// 去除PKCS7填充
std::string pkcs7_unpad(const std::string& data) {
    if (data.empty()) return "";
    char pad_len = data.back();
    if (pad_len < 1 || pad_len > AES_BLOCK_SIZE) return "";
    // 校验填充合法性（增强安全性）
    for (size_t i = data.size() - pad_len; i < data.size(); ++i) {
        if (data[i] != pad_len) return "";
    }
    return data.substr(0, data.size() - static_cast<size_t>(pad_len));
}

// 从密码生成AES-256密钥（SHA256哈希）
bool generate_aes_key(const std::string& password, unsigned char* key) {
    if (!key) return false;
    SHA256(reinterpret_cast<const unsigned char*>(password.c_str()), password.size(), key);
    return true;
}

// 生成随机IV（16字节）
bool generate_iv(unsigned char* iv) {
    if (!iv) return false;
    return RAND_bytes(iv, AES_BLOCK_SIZE) == 1;
}

/**
 * AES-256-CBC加密（使用OpenSSL 3.x EVP高级接口）
 * @param plaintext 明文（字符流）
 * @param password  密码字符串
 * @param ciphertext 输出：加密后的数据（IV + 密文）
 * @return 成功返回true，失败返回false
 */
bool aes_encrypt(const std::string& plaintext, const std::string& password, std::string& ciphertext);

/**
 * AES-256-CBC解密（使用OpenSSL 3.x EVP高级接口）
 * @param ciphertext 加密后的数据（IV + 密文）
 * @param password   密码字符串
 * @param plaintext  输出：解密后的明文
 * @return 成功返回true，失败返回false
 */
bool aes_decrypt(const std::string& ciphertext, const std::string& password, std::string& plaintext);
}

namespace data_packet
{
    template<typename Iter>
    std::pair<std::unique_ptr<byte[]>,size_t> encrypt(const Iter& begin,size_t buffer_size,const std::string& password={})
    {
        static_assert(sizeof(typename std::iterator_traits<Iter>::value_type) == sizeof(uint8_t), "Element type must be 1 byte");

        std::string plaintext(begin,std::next(begin,buffer_size));
        std::string ciphertext;
        if(!encryption::aes_encrypt(plaintext,password,ciphertext))
        {
            std::cerr<<"fail to encrypt"<<std::endl;
            return std::pair<std::unique_ptr<byte[]>,size_t>(nullptr,0);
        }
        std::unique_ptr<byte[]> encrypted_data=std::make_unique<byte[]>(ciphertext.size());
        const byte* ciphertext_tmp=ciphertext.data();
        std::memcpy(encrypted_data.get(),ciphertext_tmp,ciphertext.size());
        return std::pair<std::unique_ptr<byte[]>,size_t>(std::move(encrypted_data),ciphertext.size());
    }
    
    template<typename Iter>
    std::pair<std::unique_ptr<byte[]>,size_t> encrypt(const Iter& begin,const Iter& end,const std::string& password={})
    {
        return encrypt(begin,size_t(end-begin),password);
    }


    template<typename Iter>
    std::pair<std::unique_ptr<byte[]>,size_t> decrypt(const Iter& begin,size_t buffer_size,const std::string& password={})
    {
        static_assert(sizeof(typename std::iterator_traits<Iter>::value_type) == sizeof(uint8_t), "Element type must be 1 byte");
        std::string ciphertext(begin,std::next(begin,buffer_size));
        std::string plaintext;
        if(!encryption::aes_decrypt(ciphertext,password,plaintext))
        {
            std::cerr<<"fail to decrypt"<<std::endl;
            return std::pair<std::unique_ptr<byte[]>,size_t>(nullptr,0);
        }
        std::unique_ptr<byte[]> decrypted_data=std::make_unique<byte[]>(plaintext.size());
        const byte* plaintext_tmp=plaintext.data();
        std::memcpy(decrypted_data.get(),plaintext_tmp,plaintext.size());
        return std::pair<std::unique_ptr<byte[]>,size_t>(std::move(decrypted_data),plaintext.size());
    }
    
    template<typename Iter>
    std::pair<std::unique_ptr<byte[]>,size_t> decrypt(const Iter& begin,const Iter& end,const std::string& password={})
    {
        return decrypt(begin,size_t(end-begin),password);
    }
}