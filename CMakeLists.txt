cmake_minimum_required(VERSION 3.21)
project(WebAppClient)

set(CMAKE_CXX_STANDARD 17)

add_executable(WebAppClient main.cpp Client.cpp Client.hpp)
