cmake_minimum_required(VERSION 3.10)
project(WebServer)

include_directories(tcp)
include_directories(http)
include_directories(utils)

SET(CMAKE_CXX_FLAGS "-Wall -g -std=c++14 -Werror")
set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_FLAGS_DEBUG "-O0")

find_package(Threads REQUIRED)

aux_source_directory(utils SRC_UTIL)
aux_source_directory(tcp SRC_TCP)
aux_source_directory(http SRC_HTTP)

set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build/test/)

# add_executable(echoserver echo_server.cpp ${SRC})
# target_link_libraries(echoserver ${CMAKE_THREAD_LIBS_INIT})

add_executable(client client.cpp)

# add_executable(http_test test_httpcontext.cpp ${SRC_HTTP})

add_executable(http_server http_server.cpp ${SRC_UTIL} ${SRC_HTTP} ${SRC_TCP})
target_link_libraries(http_server ${CMAKE_THREAD_LIBS_INIT})