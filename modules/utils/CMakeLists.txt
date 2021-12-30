cmake_minimum_required (VERSION 3.8)

project(utils)

add_library(utils
    src/utils/utils.cpp
)

target_include_directories(utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/)
