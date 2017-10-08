/**
 * AlsaHardwareController.cpp
 *
 * TODO: Add Intel copyright
 */

#include <AVSCommon/Utils/Logger/Logger.h>

#include "AlsaController/AlsaHardwareController.h"

namespace alexaClientSDK {
namespace kwd {

// Logging tag
static const std::string TAG("SocketHardwareController");
#define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)

std::shared_ptr<AlsaHardwareController> AlsaHardwareController::create(
        std::string name, std::string keyword) 
{
    std::shared_ptr<AlsaHardwareController> ctrl = std::make_shared<AlsaHardwareController>(
            AlsaHardwareController(name, keyword));

    if(!ctrl->init()) {
        ACSDK_ERROR(LX("createFailed").d("reason", "initHardwareControllerFailed"));
        return nullptr;
    }

    return ctrl;
}

std::unique_ptr<KeywordDetection> AlsaHardwareController::read(
        std::chrono::milliseconds timeout) {
    // Get the file descriptor
    struct pollfd fds[1];
    snd_ctl_poll_descriptors(m_ctl, &fds[0], 1);

    int ret = poll(fds, 1, timeout.count());

    if(ret == 0) {
        // 0 indicates a timeout
        return nullptr;
    } else if(ret < 0) {
        // < 0 indicates an error occurred
        ACSDK_ERROR(LX("readFailed")
                .d("reason", "pollFailed")
                .d("error_code", std::to_string(ret)));
        return nullptr;
    }

    snd_ctl_elem_value_t* control;
    snd_ctl_elem_value_alloca(&control);
    snd_ctl_elem_value_set_interface(control, SND_CTL_ELEM_IFACE_MIXER);
    snd_ctl_elem_value_set_name(control, "KP Detect Control");
    snd_ctl_elem_value_set_index(control, 0);
    
    if((ret = snd_ctl_elem_read(m_ctl, control)) != 0) {
        ACSDK_ERROR(LX("readFailed")
                .d("reason", "sndCtrlElemReadFailed")
                .d("error_code", std::to_string(ret)));
        // Freeing control elem value
        snd_ctl_elem_value_free(control);
        return nullptr;
    }

    uint32_t value = snd_ctl_elem_value_get_integer(control, 0);
    uint16_t* payload = reinterpret_cast<uint16_t*>(&value);
    
    auto detection = KeywordDetection::create(payload[0], payload[1], m_keyword);

    // Freeing control elem value
    snd_ctl_elem_value_free(control);

    return detection;
}

AlsaHardwareController::AlsaHardwareController(std::string name, std::string keyword) :
    m_name(name), m_keyword(keyword), m_ctl(NULL)
{}

AlsaHardwareController::~AlsaHardwareController() {
    // Note: snd_ctl_close both closes the connection and frees all of its
    // allocated resources (see ALSA docs for more info)
    if(m_ctl != NULL) {
        snd_ctl_close(m_ctl);
    }
}

bool AlsaHardwareController::init() {
    int ret = 0;

    if((ret = snd_ctl_open(&m_ctl, m_name.c_str(), SND_CTL_READONLY)) < 0) {
        ACSDK_ERROR(LX("initFailed")
                .d("reason", "openAlsaCtrlFailed")
                .d("error_code", std::to_string(ret)));
        return false;
    }

    // Note: 1 tells the API to subscribe
    if((ret = snd_ctl_subscribe_events(m_ctl, 1)) < 0) {
        ACSDK_ERROR(LX("initFailed")
                .d("reason", "alsaCtrlSubscribeFailed")
                .d("error_code", std::to_string(ret)));
        return false;
    }

    return true;
}

} // kwd
} // alexaClientSDK
