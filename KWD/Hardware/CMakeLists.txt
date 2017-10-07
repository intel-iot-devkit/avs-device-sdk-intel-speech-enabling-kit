cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(HARDWARE LANGUAGES CXX)

set(HARDWARE_KWD_INCLUDE_PATH "${CMAKE_CURRENT_LIST_DIR}/include")

include(../../build/BuildDefaults.cmake)

add_subdirectory("HardwareController")
add_subdirectory("src")
add_subdirectory("test")
