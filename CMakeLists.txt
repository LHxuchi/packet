cmake_minimum_required(VERSION 4.0)

project(data_back_up)

set(CMAKE_CXX_STANDARD 20)

# 递归遍历 all .c and .cpp files
file(GLOB_RECURSE SRC
        "${CMAKE_SOURCE_DIR}/src/*.c"
        "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_library(packet SHARED ${SRC})