cmake_minimum_required(VERSION 3.21)
project(smtp_common C)

set(CMAKE_C_STANDARD 99)

add_library(smtp_common queue_msg.h helpers.c helpers.h log.c log.h)
