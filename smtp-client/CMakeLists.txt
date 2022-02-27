cmake_minimum_required(VERSION 3.21)
project(smtp_client C)

set(CMAKE_C_STANDARD 99)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic -Werror")

add_executable(smtp_client main.c ../smtp-common/log.c ../smtp-common/log.h ../smtp-common/helpers.c ../smtp-common/helpers.h)
