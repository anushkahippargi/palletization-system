cmake_minimum_required(VERSION 3.16)

project(PalletizationSystem)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_executable(palletization
    src/main.cpp
    src/Box.cpp
)