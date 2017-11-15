add_definitions("-DASCDK_LOG_MODULE=hardwareKeywordDetector")
add_library(HARDWARE SHARED HardwareKeywordDetector.cpp KeywordDetection.cpp)

target_include_directories(HARDWARE PUBLIC
    "../include"
    "../HardwareController/include"
    "${PORTAUDIO_INCLUDE_DIR}"
    "${KWD_SOURCE_DIR}/include")

target_link_libraries(HARDWARE KWD AVSCommon "${PORTAUDIO_LIB_DIR}" pthread)

# install target
asdk_install()
