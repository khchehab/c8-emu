cmake_minimum_required(VERSION 3.27)
project(c8-emu CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_COMPILER clang++)

find_package(SDL2 REQUIRED)
if (NOT SDL2_FOUND)
    message(FATAL_ERROR "SDL2 not found")
else ()
    message(STATUS "SDL2 ${SDL2_VERSION}")
    include_directories(${SDL2_INCLUDE_DIRS})
    set(LIBRARIES ${SDL2_LIBRARIES})
endif ()

include_directories(include)

add_compile_options(-O3)

file(GLOB SOURCES src/*.cpp)
add_executable(c8-emu ${SOURCES})

target_link_libraries(c8-emu ${LIBRARIES})

file(COPY roms DESTINATION ${CMAKE_BINARY_DIR})
