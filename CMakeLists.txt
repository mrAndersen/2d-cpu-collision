cmake_minimum_required(VERSION 3.28)
project(2d_collision)

set(CMAKE_CXX_STANDARD 23)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
        "src/*.h"
        "src/*.hpp"
        "src/*.cpp"
)

add_executable(
        2d_collision
        ${SOURCES}
        main.cpp
)

add_subdirectory(
        vendor/raylib
)

target_include_directories(
        2d_collision
        PRIVATE
        src
)

target_link_libraries(
        2d_collision
        raylib
)