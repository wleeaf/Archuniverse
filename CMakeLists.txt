cmake_minimum_required(VERSION 3.28)

project(Archuniverse
    VERSION 0.2.0
    DESCRIPTION "A modern C++ RPG combat engine"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ----------------------------------------------------------------------------
# Engine library (compiled static lib; grows into the full engine)
# ----------------------------------------------------------------------------
file(GLOB_RECURSE ARCH_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(archuniverse STATIC ${ARCH_SOURCES})
target_include_directories(archuniverse PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(archuniverse PUBLIC cxx_std_23)
target_compile_options(archuniverse PRIVATE
    -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion)

# ----------------------------------------------------------------------------
# Demo executables
# ----------------------------------------------------------------------------
add_executable(archuniverse_demo apps/demo/main.cpp)
target_link_libraries(archuniverse_demo PRIVATE archuniverse)
target_compile_options(archuniverse_demo PRIVATE -Wall -Wextra -Wpedantic)

find_package(Threads REQUIRED)
add_executable(archuniverse_net_demo apps/net_demo/main.cpp)
target_link_libraries(archuniverse_net_demo PRIVATE archuniverse Threads::Threads)
target_compile_options(archuniverse_net_demo PRIVATE -Wall -Wextra -Wpedantic)

# ----------------------------------------------------------------------------
# Tests
# ----------------------------------------------------------------------------
enable_testing()
add_subdirectory(tests)
