cmake_minimum_required(VERSION 3.4.0)

set(WASI_SDK_PREFIX ${CMAKE_SOURCE_DIR}/wasi-sdk-12.0)

######################
## WASI SDK Toolchain Section
######################

set(WASI TRUE)

set(WASI_CC ${WASI_SDK_PREFIX}/bin/clang)
set(WASI_CXX ${WASI_SDK_PREFIX}/bin/clang++)
set(WASI_LD ${WASI_SDK_PREFIX}/bin/wasm-ld)
set(WASI_AR ${WASI_SDK_PREFIX}/bin/llvm-ar)
set(WASI_RANLIB ${WASI_SDK_PREFIX}/bin/llvm-ranlib)
set(WASI_NM ${WASI_SDK_PREFIX}/bin/llvm-nm)

set(CMAKE_SYSTEM_NAME Generic) # Generic for now, to not trigger a Warning
set(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_SYSTEM_PROCESSOR wasm32)
set(CMAKE_SYSTEM_PROCESSOR x86)
set(CMAKE_C_COMPILER_ID Wasienv)

set(CMAKE_C_COMPILER ${WASI_CC})
set(CMAKE_CXX_COMPILER ${WASI_CXX})
set(CMAKE_LINKER ${WASI_LD} CACHE STRING "wasienv build")
set(CMAKE_AR ${WASI_AR} CACHE STRING "wasienv build")
set(CMAKE_RANLIB ${WASI_RANLIB} CACHE STRING "wasienv build")
set(CMAKE_SYSROOT ${WASI_SDK_PREFIX}/share/wasi-sysroot CACHE STRING "wasi-sdk build")

# Don't look in the sysroot for executables to run during the build
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# Only look in the sysroot (not in the host paths) for the rest
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Some other hacks
set(CMAKE_C_COMPILER_WORKS ON)
set(CMAKE_CXX_COMPILER_WORKS ON)

set(CMAKE_C_FLAGS "-g -fno-exceptions" CACHE STRING "wasi-sdk build")
set(CMAKE_CXX_FLAGS "-g -std=c++17 -fno-exceptions" CACHE STRING "wasi-sdk build")
set(CMAKE_EXE_LINKER_FLAGS "-g -fno-exceptions -nostartfiles -Wl,--no-entry -Wl,--import-memory -Wl,--export-dynamic")

######################
## Project Section
######################

project(wasm-quickjs-loader CXX)

add_subdirectory(modules/3rdparty/quickjs)
add_subdirectory(modules/3rdparty/nanopb)
add_subdirectory(modules/3rdparty/json)
add_subdirectory(modules/io)
add_subdirectory(modules/js)
add_subdirectory(modules/utils)

add_executable(wasm-quickjs-loader
    src/entrypoint.cpp
    src/main.cpp
)

target_link_libraries(wasm-quickjs-loader PUBLIC io JS utils)