cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(HARDWARE_CONTROLLER LANGUAGES CXX)

include(../../../build/BuildDefaults.cmake)

add_subdirectory("src")

if(SOCKET_HARDWARE_CONTROLLER)
    add_subdirectory("Socket")
endif()
