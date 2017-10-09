add_executable(StressTesterApp 
    main.cpp
    Arguments.cpp
    StressTesterController.cpp
    AudioFilePlayer.cpp)

target_include_directories(StressTesterApp PUBLIC
    "../include"
    "${MediaPlayer_SOURCE_DIR}/include"
    "${DefaultClient_SOURCE_DIR}/include")

target_link_libraries(StressTesterApp rt m pthread HARDWARE)
