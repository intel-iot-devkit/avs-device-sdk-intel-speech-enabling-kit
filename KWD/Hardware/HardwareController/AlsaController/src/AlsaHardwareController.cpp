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
static const std::string TAG("AlsaHardwareController");
#define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)

// ALSA control device name for receving keyphrase detection events
#define CTL_DETECT_NAME "KP Detect Control"
#define CTL_CAP_STREAM_MODE "Capture Stream mode"
#define CTL_DSP_TOPO "DSP Load Topology Control"

// DSP unload/load values
#define DSP_UNLOAD 0
#define DSP_LOAD   1

// Multi-Turn modes
#define KP_WAKE_ON_VOICE     0
#define KP_CAPTURE_STREAMING 1

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
                .d("error_code", snd_strerror(ret)));
        return nullptr;
    }

    snd_ctl_event_t* event;
    snd_ctl_event_alloca(&event);
    if(snd_ctl_read(m_ctl, event) < 0) {
        ACSDK_ERROR(LX("readFailed")
                .d("reason", "sndReadFailed")
                .d("error_code", snd_strerror(ret)));
        return nullptr;
    }

    // Get reference to the KP detect control device
    snd_ctl_elem_value_t* control = createSndMixerCtl(CTL_DETECT_NAME);
    
    if((ret = snd_ctl_elem_read(m_ctl, control)) != 0) {
        ACSDK_ERROR(LX("readFailed")
                .d("reason", "sndCtrlElemReadFailed")
                .d("error_code", snd_strerror(ret)));
        return nullptr;
    }

    uint32_t value = snd_ctl_elem_value_get_integer(control, 0);
    uint16_t* payload = reinterpret_cast<uint16_t*>(&value);
    
    auto detection = KeywordDetection::create(payload[0], payload[1], m_keyword);

    ACSDK_DEBUG9(LX("read")
            .d("event", "keywordDetection")
            .d("begin", payload[0])
            .d("end", payload[1]));

    return detection;
}

void AlsaHardwareController::onStateChanged(AipState state) {
    if(state == AipState::EXPECTING_SPEECH) {
        ACSDK_DEBUG9(LX("onStateChanged").d("event", "setCaptureStremingMode"));
        int ret = 0;
        // Create handle to the streaming mode control device 
        snd_ctl_elem_value_t* control = createSndMixerCtl(CTL_CAP_STREAM_MODE);
        // Set mode to streaming mode
        snd_ctl_elem_value_set_integer(control, 0, KP_CAPTURE_STREAMING);
        if((ret = snd_ctl_elem_write(m_ctl, control)) != 0) {
            ACSDK_ERROR(LX("writeFailed")
                    .d("reason", "setCaptureStreamModeFailed")
                    .d("error_code", snd_strerror(ret)));
        }
    } else if(m_currentAipState == AipState::EXPECTING_SPEECH) {
        ACSDK_DEBUG9(LX("onStateChanged").d("event", "setWakeOnVoiceMode"));
        int ret = 0;
        // Create handle to the streaming mode control device 
        snd_ctl_elem_value_t* control = createSndMixerCtl(CTL_CAP_STREAM_MODE);
        // Set mode to streaming mode
        snd_ctl_elem_value_set_integer(control, 0, KP_WAKE_ON_VOICE);
        if((ret = snd_ctl_elem_write(m_ctl, control)) != 0) {
            ACSDK_ERROR(LX("writeFailed")
                    .d("reason", "setWakeOnVoiceModeFaile")
                    .d("error_code", snd_strerror(ret)));
        }
    }
    m_currentAipState = state;
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
                .d("error_code", snd_strerror(ret)));
        return false;
    }

    // Note: 1 tells the API to subscribe
    if((ret = snd_ctl_subscribe_events(m_ctl, 1)) < 0) {
        ACSDK_ERROR(LX("initFailed")
                .d("reason", "alsaCtrlSubscribeFailed")
                .d("error_code", snd_strerror(ret)));
        return false;
    }

    // Create handle to the DSP topology control device
    snd_ctl_elem_value_t* control = createSndMixerCtl(CTL_DSP_TOPO);
    
    // Unload the DSP's topology
    snd_ctl_elem_value_set_integer(control, 0, DSP_UNLOAD);
    if((ret = snd_ctl_elem_write(m_ctl, control)) != 0) {
        ACSDK_ERROR(LX("writeFailed")
                .d("reason", "dspUnloadFailed")
                .d("error_code", snd_strerror(ret)));
        return false;
    }

    // Load the DSP's topology
    snd_ctl_elem_value_set_integer(control, 0, DSP_LOAD);
    if((ret = snd_ctl_elem_write(m_ctl, control)) != 0) {
        ACSDK_ERROR(LX("writeFailed")
                .d("reason", "dspLoadFailed")
                .d("error_code", snd_strerror(ret)));
        return false;
    }

    return true;
}

snd_ctl_elem_value_t* AlsaHardwareController::createSndMixerCtl(
        const char* name, int index) 
{
    snd_ctl_elem_value_t* control;
    snd_ctl_elem_value_alloca(&control);
    snd_ctl_elem_value_set_interface(control, SND_CTL_ELEM_IFACE_MIXER);
    snd_ctl_elem_value_set_name(control, name);
    snd_ctl_elem_value_set_index(control, index);
    return control;
}

} // kwd
} // alexaClientSDK
