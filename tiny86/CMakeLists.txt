cmake_minimum_required(VERSION 3.5)

set(PROJECT_NAME "libt86")

project(${PROJECT_NAME})
file(GLOB_RECURSE SRC "*.cpp" "*.h")
add_library(${PROJECT_NAME} ${SRC})
