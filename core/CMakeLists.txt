cmake_minimum_required(VERSION 3.16)

project(PalletizationSystem)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_executable(palletization
    src/main.cpp
    src/Box.cpp
    src/Pallet.cpp
    src/Placement.cpp
    src/Statistics.cpp
    src/RowAlgorithm.cpp
    src/Matrix4x4.cpp
)