cmake_minimum_required(VERSION 3.5)

project(data_back_up)

# 设置可执行文件输出路径
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

set(LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib")

set(CMAKE_CXX_STANDARD 20)

# 递归遍历 all .c and .cpp files
file(GLOB_RECURSE SRC
        "${CMAKE_SOURCE_DIR}/src/*.c"
        "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_library(packet SHARED ${SRC})

# 在测试目标定义后添加
set_target_properties(packet PROPERTIES
        CMAKE_BUILD_TYPE Debug
        COMPILE_FLAGS "-g -O0"  # -g 生成调试信息，-O0 关闭优化
)

add_subdirectory("${CMAKE_SOURCE_DIR}/test")