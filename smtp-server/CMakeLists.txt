cmake_minimum_required(VERSION 3.21)
project(smtp_server C)

include_directories(/usr/local/include)
link_directories(/usr/local/lib)

find_library(libconfig libconfig)

set(CMAKE_C_STANDARD 99)

file(GLOB CODE_FILES *.c)

include_directories(${smtp_common_INCLUDE_DIRS})

add_executable(smtp_server ${CODE_FILES})

target_link_libraries(smtp_server smtp_common config)


