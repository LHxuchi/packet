# 构建测试程序

# 查找 Catch2 库
find_package(Catch2 3 REQUIRED)

# 查找 OpenSSL 库（关键：必须先查找，再链接）
find_package(OpenSSL REQUIRED)
if (OPENSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIR})
    message(STATUS "OpenSSL found: ${OPENSSL_VERSION}")
else()
    message(FATAL_ERROR "OpenSSL not found! Please install libssl-dev.")
endif()

include_directories("/include")

aux_source_directory("${CMAKE_SOURCE_DIR}/test/src" test_files)

# 添加测试目标
add_executable(utest ${test_files})

add_dependencies(utest packet)

# 链接库：测试程序需要链接待测试的库和 Catch2
target_link_libraries(utest PRIVATE packet Catch2::Catch2WithMain
                        ${OPENSSL_CRYPTO_LIBRARY}  # 链接 libcrypto.so（包含 SHA256/AES 等）
                        ${OPENSSL_SSL_LIBRARY}  # 若用到 SSL 相关功能，需添加
)

# 在测试目标定义后添加
set_target_properties(utest PROPERTIES
        CMAKE_BUILD_TYPE Debug
        COMPILE_FLAGS "-g -O0"  # -g 生成调试信息，-O0 关闭优化
)