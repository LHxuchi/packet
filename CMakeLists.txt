cmake_minimum_required(VERSION 3.5)
project(data_back_up)

# 基础配置
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")  # 可执行文件输出到bin
set(LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib")        # 库文件输出到lib

# -------------------------- 1. 查找依赖 --------------------------
# 查找OpenSSL（加密依赖）
find_package(OpenSSL REQUIRED)
if(NOT OPENSSL_FOUND)
    message(FATAL_ERROR "OpenSSL not found! Install libssl-dev (Ubuntu) or openssl-devel (CentOS)")
endif()

# 查找Qt5（GUI依赖，必须安装Qt5开发包）
find_package(Qt5 COMPONENTS Widgets REQUIRED)
if(NOT Qt5_FOUND)
    message(FATAL_ERROR "Qt5 Widgets not found! Install qtbase5-dev (Ubuntu) or qt5-qtbase-devel (CentOS)")
endif()

# -------------------------- 2. 收集源文件 --------------------------
# 收集packet库的源文件（原有逻辑）
file(GLOB_RECURSE PACKET_SRC
    "${CMAKE_SOURCE_DIR}/src/*.c"
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

# 收集GUI源文件
file(GLOB GUI_SRC
    "${CMAKE_SOURCE_DIR}/GUI/*.cpp"
)

# -------------------------- 3. 构建packet共享库 --------------------------
add_library(packet SHARED ${PACKET_SRC})

# 配置packet库的头文件路径
target_include_directories(packet
    PUBLIC  # 公开头文件，让依赖packet的目标（如GUI）能包含
    "${CMAKE_SOURCE_DIR}/include"
    PRIVATE # 私有依赖（仅packet自身使用）
    ${OPENSSL_INCLUDE_DIR}
    ${Qt5Widgets_INCLUDE_DIRS}
)

# 链接依赖到packet库
target_link_libraries(packet
    PRIVATE
    OpenSSL::Crypto  # OpenSSL加密库
    Qt5::Widgets     # Qt5界面库（若packet自身不用Qt，可只链接到GUI可执行文件）
)

# 设置packet库的调试属性
set_target_properties(packet PROPERTIES
    CMAKE_BUILD_TYPE Debug
    COMPILE_FLAGS "-g -O0"
)

# -------------------------- 4. 构建GUI可执行文件 --------------------------
add_executable(backup_gui ${GUI_SRC})

# 配置GUI的头文件路径
target_include_directories(backup_gui
    PRIVATE
    "${CMAKE_SOURCE_DIR}/include"  # 项目头文件
    ${Qt5Widgets_INCLUDE_DIRS}     # Qt5头文件
)

# 链接依赖到GUI可执行文件（核心：链接packet库+Qt5+OpenSSL）
target_link_libraries(backup_gui
    PRIVATE
    packet          # 项目核心库
    Qt5::Widgets    # Qt5界面库
    OpenSSL::Crypto # OpenSSL加密库
)

# -------------------------- 5. Qt5 MOC编译（必须） --------------------------
# 自动处理Qt的Q_OBJECT宏（生成moc文件）
set_target_properties(backup_gui PROPERTIES
    AUTOMOC ON  # 自动运行moc工具
    AUTOUIC ON  # 若用.ui文件需开启，这里纯代码无需
    AUTORCC ON  # 若有资源文件需开启，这里无需
)

# -------------------------- 6. 保留原有测试目录 --------------------------
add_subdirectory("${CMAKE_SOURCE_DIR}/test")
