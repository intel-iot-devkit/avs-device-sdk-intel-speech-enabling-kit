add_definitions("-DASCDK_LOG_MODULE=alsaHardwareController")
add_library(ALSA_HW_CONTROLLER SHARED AlsaHardwareController.cpp)

target_include_directories(ALSA_HW_CONTROLLER PUBLIC
    "../include/"
    "../../include"
    "${HARDWARE_KWD_INCLUDE_PATH}")

target_link_libraries(ALSA_HW_CONTROLLER AVSCommon HARDWARE)

# install target
asdk_install()
