cmake_minimum_required(VERSION 3.4.0)

#set(CMAKE_VERBOSE_MAKEFILE ON)
set(WASI 1)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(wasm-quickjs-loader CXX)

add_subdirectory(modules/3rdparty/quickjs)

add_subdirectory(modules/3rdparty/nanopb)

add_subdirectory(modules/io)

add_subdirectory(modules/js)

add_executable(wasm-quickjs-loader
    src/entrypoint.cpp
)

target_link_libraries(wasm-quickjs-loader PUBLIC io JS)