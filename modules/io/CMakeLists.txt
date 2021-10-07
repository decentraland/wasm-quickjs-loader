cmake_minimum_required (VERSION 3.8)

project(io)

add_library(io
    src/io/ChannelIOModule.cpp
    src/io/FDChannel.cpp
)

target_include_directories(io PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/)
