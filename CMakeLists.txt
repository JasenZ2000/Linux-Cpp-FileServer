cmake_minimum_required(VERSION 3.10)
project(WebServer)

include_directories(tcp)

SET(CMAKE_CXX_FLAGS "-Wall -g -std=c++14 -Werror")
set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_FLAGS_DEBUG "-O0")

find_package(Threads REQUIRED)

aux_source_directory(tcp SRC)

set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build/test/)

add_executable(echoserver echo_server.cpp ${SRC})
target_link_libraries(echoserver ${CMAKE_THREAD_LIBS_INIT})

add_executable(client client.cpp)