
project(nlohmannjson)

add_library(nlohmannjson
    INTERFACE
)
target_include_directories(nlohmannjson SYSTEM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include/)
