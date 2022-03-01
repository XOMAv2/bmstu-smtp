cmake_minimum_required(VERSION 3.21)
project(smtp_common C)

set(CMAKE_C_STANDARD 99)

file(GLOB CODE_FILES *.c)

add_library(smtp_common STATIC ${CODE_FILES})

set(${PROJECT_NAME}_INCLUDE_DIRS ${PROJECT_SOURCE_DIR} CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE)
