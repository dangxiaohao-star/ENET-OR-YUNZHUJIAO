cmake_minimum_required(VERSION 3.20.0)

project(EdoyunNet LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
# 如果编译器不支持 C++11，CMake 会在配置阶段直接报错
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${PROJECT_SOURCE_DIR}/EdoyunNet)

# exe输出路径（老式写法）
# set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# exe输出路径 现代写法
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

# 资源文件路径(旧写法)
# aux_source_directory(${PROJECT_SOURCE_DIR}/EdoyunNet SRC_LIST)

# 现代CMake：使用带有 CONFIGURE_DEPENDS 的 file(GLOB)
# CONFIGURE_DEPENDS 会让构建系统在每次 make/ninja 时检查目录是否有文件增删，如果有，会自动重新运行 CMake！
file(GLOB SRC_LIST CONFIGURE_DEPENDS
    ${PROJECT_SOURCE_DIR}/EdoyunNet/*.cpp
)

add_executable(
    EdoyunNet
    ${SRC_LIST}
)

target_include_directories(EdoyunNet PRIVATE
    ${PROJECT_SOURCE_DIR}/EdoyunNet
)