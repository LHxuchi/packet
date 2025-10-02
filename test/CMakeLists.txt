# 构建测试程序


# 查找 Catch2 库
find_package(Catch2 3 REQUIRED)

include_directories("/include")

aux_source_directory("${CMAKE_SOURCE_DIR}/test" test_files)

# 添加测试目标
add_executable(utest ${test_files})

# 链接库：测试程序需要链接待测试的库和 Catch2
target_link_libraries(utest PRIVATE packet Catch2::Catch2WithMain)