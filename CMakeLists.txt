cmake_minimum_required(VERSION 3.21)
project(bmstu_smtp C)

include_directories(/usr/local/include)
link_directories(/usr/local/lib)

find_library(libconfig libconfig)

add_subdirectory("${PROJECT_SOURCE_DIR}/smtp-client")
add_subdirectory("${PROJECT_SOURCE_DIR}/smtp-server")
add_subdirectory("${PROJECT_SOURCE_DIR}/smtp-common")

add_subdirectory("${PROJECT_SOURCE_DIR}/smtp-server-test")