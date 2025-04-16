cmake_minimum_required(VERSION 3.31)
project(Redis-Build)

set(CMAKE_CXX_STANDARD 20)

add_executable(Redis_Server server.cpp)
add_executable(Redis_Client client.cpp)
