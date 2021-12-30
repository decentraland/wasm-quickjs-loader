cmake_minimum_required (VERSION 3.8)

project(nanopb C)

add_library(nanopb
    src/pb_common.c
    src/pb_decode.c
    src/pb_encode.c
)

target_include_directories(nanopb PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/)
