set(INCLUDES 
    "${CMAKE_CURRENT_LIST_DIR}/../include/"
    "${PORTAUDIO_INCLUDE_DIR}" 
    "${MediaPlayer_SOURCE_DIR}/include"
    "${GST_INCLUDE_DIRS}"
    "${KWD_SOURCE_DIR}/include")

set(LIBRARIES 
    HARDWARE
    MediaPlayer
    ${CMAKE_THREAD_LIBS_INIT})


if(SOCKET_HARDWARE_CONTROLLER)
    list(APPEND INCLUDES "../HardwareController/Socket/include")
    list(APPEND LIBRARIES SOCKET)
endif()

if(ALSA_HARDWARE_CONTROLLER)
    list(APPEND INCLUDES "../HardwareController/AlsaController/include")
    list(APPEND LIBRARIES ALSA_HW_CONTROLLER)
endif()

find_package(Threads ${THREADS_PACKAGE_CONFIG})

set(INPUTFOLDER "${KWD_SOURCE_DIR}/inputs")

discover_unit_tests("${INCLUDES}" "${LIBRARIES}" "${INPUTFOLDER}")
