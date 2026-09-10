cmake_minimum_required(VERSION 3.20)

project(NanoBoy
    VERSION 0.1
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(NanoBoy
    src/main.cpp
    src/cpu/cpu.cpp
    src/memory/mem.cpp
)