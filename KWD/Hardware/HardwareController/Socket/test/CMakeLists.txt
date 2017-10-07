
add_executable(socket_ctrl_test SocketHardwareControllerTest.cpp)
target_include_directories(socket_ctrl_test PUBLIC
    "../include/"
    "../../include"
    "${HARDWARE_KWD_INCLUDE_PATH}")

target_link_libraries(socket_ctrl_test AVSCommon SOCKET)
