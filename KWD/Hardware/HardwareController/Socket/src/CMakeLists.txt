add_definitions("-DASCDK_LOG_MODULE=socketHardwareController")
add_library(SOCKET SHARED SocketHardwareController.cpp)

target_include_directories(SOCKET PUBLIC
    "../include/"
    "../../include"
    "${HARDWARE_KWD_INCLUDE_PATH}")

target_link_libraries(SOCKET AVSCommon HARDWARE)

# install target
asdk_install()
