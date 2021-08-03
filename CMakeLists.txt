cmake_minimum_required(VERSION 3.20)

project(DXRSample)

set(CMAKE_CXX_STANDARD 17)

find_package(Threads REQUIRED)

add_subdirectory(Lib)

add_subdirectory(Test)