# Cmake toolchain description file for the Makefile for WASI
cmake_minimum_required(VERSION 3.4.0)

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

# We set the cross compiler using the WebAssembly VM
# set(CMAKE_CROSSCOMPILING TRUE)
# if (NOT DEFINED CMAKE_CROSSCOMPILING_EMULATOR)
#   find_program(WASIRUN_EXECUTABLE NAMES wasirun)
#   if(WASIRUN_EXECUTABLE)
#     set(CMAKE_CROSSCOMPILING_EMULATOR "wasirun" CACHE FILEPATH "Path to the emulator for the target system.")
#   endif()
# endif()
# if(CMAKE_CROSSCOMPILING_EMULATOR)
# endif()


# Not needed now (but perhaps in the future)

# set(CMAKE_EXECUTABLE_SUFFIX .wasm)
# set(CMAKE_EXECUTABLE_SUFFIX_C .wasm)
# set(CMAKE_EXECUTABLE_SUFFIX_CXX .wasm)
# set(triple wasm32-wasi)

# set(WASI_SDK_PREFIX $ENV{WASI_SDK_DIR})
# set(CMAKE_C_COMPILER ${WASI_SDK_PREFIX}/bin/clang)
# set(CMAKE_CXX_COMPILER ${WASI_SDK_PREFIX}/bin/clang++)
# set(CMAKE_LINKER ${WASI_SDK_PREFIX}/bin/wasm-ld CACHE STRING "wasi-sdk build")
# set(CMAKE_AR ${WASI_SDK_PREFIX}/bin/llvm-ar CACHE STRING "wasi-sdk build")
# set(CMAKE_RANLIB ${WASI_SDK_PREFIX}/bin/llvm-ranlib CACHE STRING "wasi-sdk build")

# set(CMAKE_C_COMPILER_TARGET ${triple} CACHE STRING "wasi-sdk build")
# set(CMAKE_CXX_COMPILER_TARGET ${triple} CACHE STRING "wasi-sdk build")
# set(CMAKE_C_FLAGS "-v" CACHE STRING "wasienv build")
# set(CMAKE_CXX_FLAGS "-v -std=c++11" CACHE STRING "wasienv build")

# set(CMAKE_SYSROOT ${WASI_SDK_PREFIX}/share/wasi-sysroot CACHE STRING "wasienv build")
# set(CMAKE_STAGING_PREFIX ${WASI_SDK_PREFIX}/share/wasi-sysroot CACHE STRING "wasienv build")