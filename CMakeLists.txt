cmake_minimum_required(VERSION 3.20.0)

project(EdoyunNet)

set(CMAKE_CXX_STANDARD 11)

include_directories(${PROJECT_SOURCE_DIR}/EdoyunNet)

# exe输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 资源文件路径
aux_source_directory(${PROJECT_SOURCE_DIR}/EdoyunNet SRC_LIST)

add_executable(
    EdoyunNet
    ${SRC_LIST}
)
