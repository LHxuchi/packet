cmake_minimum_required(VERSION 3.5)

project(data_back_up)

# 设置可执行文件输出路径
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

set(LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib")

set(CMAKE_CXX_STANDARD 20)

# -------------------------- 新增：查找OpenSSL --------------------------
find_package(OpenSSL REQUIRED)
if(NOT OPENSSL_FOUND)
    message(FATAL_ERROR "OpenSSL not found! Please install OpenSSL development libraries.")
endif()

# 递归遍历 all .c and .cpp files
file(GLOB_RECURSE SRC
        "${CMAKE_SOURCE_DIR}/src/*.c"
        "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_library(packet SHARED ${SRC})

# -------------------------- 新增：链接OpenSSL到packet库 --------------------------
target_link_libraries(packet
    PRIVATE
    OpenSSL::Crypto  # 加密库（对应libcrypto，AES/SHA256等功能）
    OpenSSL::SSL     # 可选：SSL库（若用到SSL功能）
)

# 在测试目标定义后添加
set_target_properties(packet PROPERTIES
        CMAKE_BUILD_TYPE Debug
        COMPILE_FLAGS "-g -O0"  # -g 生成调试信息，-O0 关闭优化
)

add_subdirectory("${CMAKE_SOURCE_DIR}/test")