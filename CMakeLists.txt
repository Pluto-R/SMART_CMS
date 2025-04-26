cmake_minimum_required(VERSION 3.10)
project(EduMatchSystem VERSION 1.0 LANGUAGES CXX)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 设置调试选项
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-g -O0 -Wall -Wextra -pedantic)
    message(STATUS "Building in DEBUG mode")
else()
    add_compile_options(-O2)
    message(STATUS "Building in RELEASE mode")
endif()

# # 查找 SQLite3
find_package(OpenSSL REQUIRED)

# 包含头文件目录
include_directories(${CMAKE_SOURCE_DIR}/include)

# 收集源文件
set(SOURCES
    src/main.cpp
    src/user_manage.cpp
    src/matching.cpp
    src/course_schedule.cpp
    src/analysis.cpp
)

# 生成可执行文件
add_executable(EduMatchSystem ${SOURCES})

# # 链接 SQLite3 库
# target_link_libraries(EduMatchSystem PRIVATE SQLite3::SQLite3)
# 链接 OpenSSL 库
target_link_libraries(EduMatchSystem PRIVATE OpenSSL::SSL OpenSSL::Crypto)


# 设置编译选项
target_compile_options(EduMatchSystem PRIVATE -Wall -Wextra)

# 安装目标（可选）
install(TARGETS EduMatchSystem DESTINATION bin)