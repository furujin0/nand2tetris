cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)

project(Compiler)

include_directories(${PROJECT_SOURCE_DIR})

add_executable(
    Compiler
    compiler.hpp
    compiler.cpp
    main.cpp
    privdef.hpp
)