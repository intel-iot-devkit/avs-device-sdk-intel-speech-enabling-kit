add_executable(StressTesterApp 
    main.cpp
    Arguments.cpp
    StressTesterController.cpp
    AudioFilePlayer.cpp
    ConnectionObserver.cpp
    KeywordObserver.cpp
    StressTesterApp.cpp)

target_include_directories(StressTesterApp PUBLIC
    "../include"
    "${MediaPlayer_SOURCE_DIR}/include"
    "${DefaultClient_SOURCE_DIR}/include")

target_link_libraries(StressTesterApp 
    DefaultClient
    AuthDelegate 
    MediaPlayer
    HARDWARE
    pthread)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_link_libraries(StressTesterApp
        "-framework CoreAudio" 
        "-framework AudioToolbox" 
        "-framework AudioUnit" 
        "-framework CoreServices" 
        "-framework Carbon")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    target_link_libraries(StressTesterApp
        rt m pthread asound)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    message(FATAL_ERROR "Windows is unsupported at the moment")  
endif()
