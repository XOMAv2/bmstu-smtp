cmake_minimum_required(VERSION 3.21)
project(smtp_server C)

include_directories(/usr/local/include)
link_directories(/usr/local/lib)

find_library(libconfig libconfig)

set(CMAKE_C_STANDARD 99)

add_executable(smtp_server main.c config.c server-fsm.c error.c smtp.c parse.c destruction.c config.h server-fsm.h error.h defines.h smtp.h parse.h destruction.h)

target_link_libraries(smtp_server config)


