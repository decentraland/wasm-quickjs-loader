cmake_minimum_required (VERSION 3.8)

project(quickjs)

add_definitions(-DEMSCRIPTEN)
add_definitions(-DQUICKJS_WITH_DEBUGGER)
add_definitions(-DCONFIG_BIGNUM)
add_definitions(-DFE_DOWNWARD=0x400)
add_definitions(-DFE_UPWARD=0x400)
add_definitions(-DCONFIG_VERSION="2021-03-27")
add_definitions(-D_WASI_EMULATED_SIGNAL)
add_definitions(-DGOOGLE_PROTOBUF_NO_THREADLOCAL)

add_library(quickjs
    src/quickjs/cutils.c
    src/quickjs/libbf.c
    src/quickjs/libregexp.c
    src/quickjs/libunicode.c
    src/quickjs/quickjs-debugger.c
    src/quickjs/quickjs-debugger-transport-wasi.c
    src/quickjs/quickjs.c
)

target_include_directories(quickjs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/)
