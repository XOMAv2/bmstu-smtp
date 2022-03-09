cmake_minimum_required(VERSION 3.21)
project(smtp_client C)

set(CMAKE_C_STANDARD 99)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic -Werror")

include_directories(${smtp_common_INCLUDE_DIRS})

file(GLOB CODE_FILES *.c)

add_executable(smtp_client ${CODE_FILES})

target_link_libraries(smtp_client smtp_common config)
