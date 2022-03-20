cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_SOURCE_DIR})

add_subdirectory(common)
add_subdirectory(tinyC)

add_subdirectory(applications/ni-gen)

project(tiny)
