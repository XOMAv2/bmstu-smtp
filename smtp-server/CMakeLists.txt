cmake_minimum_required(VERSION 3.21)
project(smtp_server C)

set(CMAKE_C_STANDARD 99)

file(GLOB CODE_FILES *.c)

include_directories(${smtp_common_INCLUDE_DIRS})

add_executable(smtp_server ${CODE_FILES})

add_library(smtp_server_lib STATIC ${CODE_FILES})

target_link_libraries(smtp_server smtp_common config)
