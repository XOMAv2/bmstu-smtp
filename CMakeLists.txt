cmake_minimum_required(VERSION 3.21)
project(bmstu_smtp C)

add_subdirectory("${PROJECT_SOURCE_DIR}/smtp-client")
add_subdirectory("${PROJECT_SOURCE_DIR}/smtp-server")
add_subdirectory("${PROJECT_SOURCE_DIR}/smtp-common")