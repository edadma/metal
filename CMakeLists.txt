cmake_minimum_required(VERSION 3.31)
project(metal C)

set(CMAKE_C_STANDARD 23)

file(GLOB_RECURSE SRC src/*.c)
add_executable(metal ${SRC})

target_include_directories(metal
        PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)
