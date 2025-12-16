#include "../../include/encryption_method/encryption.h"
bool encryption::aes_encrypt(const std::string& plaintext, const std::string& password, std::string& ciphertext) 
{
    ERR_clear_error(); // 操作前清空错误栈，避免残留错误干扰
    // 1. 生成密钥和IV
    unsigned char key[AES_KEY_SIZE_256] = {0};
    unsigned char iv[AES_BLOCK_SIZE] = {0};
    if (!generate_aes_key(password, key) || !generate_iv(iv)) {
        std::cerr << "Failed to generate key/iv" << std::endl;
        return false;
    }

    // 2. 初始化EVP加密上下文
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        std::cerr << "Failed to create EVP context" << std::endl;
        return false;
    }

    // 3. 初始化AES-256-CBC加密
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key, iv) != 1) {
        std::cerr << "Failed to init encrypt context" << std::endl;
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    // 4. 明文填充
    std::string padded_plaintext = pkcs7_pad(plaintext, AES_BLOCK_SIZE);

    // 5. 执行加密
    ciphertext.clear();
    ciphertext.resize(AES_BLOCK_SIZE + padded_plaintext.size() + AES_BLOCK_SIZE); // 预留IV+加密缓冲区
    memcpy(&ciphertext[0], iv, AES_BLOCK_SIZE); // IV放在密文开头

    int out_len = 0;
    int total_out_len = 0;
    // 加密数据
    if (EVP_EncryptUpdate(ctx, 
                          reinterpret_cast<unsigned char*>(&ciphertext[AES_BLOCK_SIZE]), 
                          &out_len, 
                          reinterpret_cast<const unsigned char*>(padded_plaintext.c_str()), 
                          padded_plaintext.size()) != 1) {
        std::cerr << "Failed to encrypt data" << std::endl;
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    total_out_len += out_len;

    // 完成加密（处理最后一块）
    if (EVP_EncryptFinal_ex(ctx, 
                            reinterpret_cast<unsigned char*>(&ciphertext[AES_BLOCK_SIZE + total_out_len]), 
                            &out_len) != 1) {
        std::cerr << "Failed to finalize encryption" << std::endl;
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    total_out_len += out_len;

    // 调整密文长度
    ciphertext.resize(AES_BLOCK_SIZE + total_out_len);
    // 清理上下文
    EVP_CIPHER_CTX_free(ctx);
    CHECK_OPENSSL_ERR(EVP_Encrypt);
    return true;
}

bool encryption::aes_decrypt(const std::string& ciphertext, const std::string& password, std::string& plaintext) {
    ERR_clear_error(); // 操作前清空错误栈，避免残留错误干扰
    // 1. 校验密文长度
    if (ciphertext.size() < AES_BLOCK_SIZE) {
        std::cerr << "Invalid ciphertext length" << std::endl;
        return false;
    }

    // 2. 提取IV并生成密钥
    unsigned char iv[AES_BLOCK_SIZE] = {0};
    memcpy(iv, &ciphertext[0], AES_BLOCK_SIZE);

    unsigned char key[AES_KEY_SIZE_256] = {0};
    if (!generate_aes_key(password, key)) {
        std::cerr << "Failed to generate key" << std::endl;
        return false;
    }

    // 3. 初始化EVP解密上下文
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        std::cerr << "Failed to create EVP context" << std::endl;
        return false;
    }

    // 4. 初始化AES-256-CBC解密
    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key, iv) != 1) {
        std::cerr << "Failed to init decrypt context" << std::endl;
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    // 5. 执行解密
    plaintext.clear();
    plaintext.resize(ciphertext.size() - AES_BLOCK_SIZE + AES_BLOCK_SIZE); // 预留解密缓冲区

    int out_len = 0;
    int total_out_len = 0;
    // 解密数据
    if (EVP_DecryptUpdate(ctx, 
                          reinterpret_cast<unsigned char*>(&plaintext[0]), 
                          &out_len, 
                          reinterpret_cast<const unsigned char*>(&ciphertext[AES_BLOCK_SIZE]), 
                          ciphertext.size() - AES_BLOCK_SIZE) != 1) {
        std::cerr << "Failed to decrypt data" << std::endl;
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    total_out_len += out_len;

    // 完成解密（处理最后一块）
    if (EVP_DecryptFinal_ex(ctx, 
                            reinterpret_cast<unsigned char*>(&plaintext[total_out_len]), 
                            &out_len) != 1) {
        std::cerr << "Failed to finalize decryption (wrong password or corrupted data)" << std::endl;
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    total_out_len += out_len;

    // 调整明文长度并去除填充
    plaintext.resize(total_out_len);
    plaintext = pkcs7_unpad(plaintext);

    // 清理上下文
    EVP_CIPHER_CTX_free(ctx);
    CHECK_OPENSSL_ERR(EVP_Decrypt);
    return true;
}
