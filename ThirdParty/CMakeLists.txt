cmake_minimum_required(VERSION 3.23)
project(LibUE)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_library(LibUE STATIC
        src/lib_ue.cpp
)

target_include_directories(LibUE PUBLIC include)
