cmake_minimum_required(VERSION 3.5)

project(terraria-chest-inspector LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_COMPILER CACHE STRING "clang")

add_executable(terraria-chest-inspector main.cpp itemNamesById.cpp prefixNamesById.cpp)

install(TARGETS terraria-chest-inspector
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
