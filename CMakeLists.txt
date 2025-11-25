cmake_minimum_required(VERSION 3.20)
project(HackerSim VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Disable char8_t to allow u8"" literals as const char*
if(MSVC)
    add_compile_options(/Zc:char8_t-)
else()
    add_compile_options(-fno-char8_t)
endif()

# UTF-8 source and execution encoding
if(MSVC)
    add_compile_options(/utf-8)
endif()
set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")

find_package(raylib CONFIG REQUIRED)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/game.cpp
    src/graphics.cpp
    src/story.cpp
    src/hacking.cpp
    src/systems.cpp
    src/types.hpp
    src/hacking.hpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE raylib)
