cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)

project(VmTranslator)

add_executable(
    VmTranslator
    VmTranslator.hpp
    VmTranslator.cpp
    main.cpp
)