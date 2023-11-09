cmake_minimum_required(VERSION 3.27)

project(yave LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(main main.cpp n/core/String.cpp n/core/utils.cpp)
target_include_directories(main PUBLIC .)
