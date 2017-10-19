cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(StressTesterApp LANGUAGES CXX)

include(../build/BuildDefaults.cmake)

if(STRESS_TESTER_APP)
    if(NOT HARDWARE_KEY_WORD_DETECTOR)
        message(FATAL_ERROR "The StressTesterApp requires the HardwareKeywordDetector")
    endif()

    # if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    #     message(FATAL_ERROR "The StressTesterApp currently only works on Linux")
    # endif()

    add_subdirectory("src")
endif()
