cmake_minimum_required (VERSION 3.8)

project(JS)

add_library(JS
  src/js/JS.cpp
  src/js/DecentralandInterface.cpp
)

target_include_directories(JS PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/)
target_link_libraries(JS PUBLIC io quickjs nlohmannjson)