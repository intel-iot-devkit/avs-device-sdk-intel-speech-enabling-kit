/**
 * StopPortAudioStreamObserver.cpp
 *
 * TODO: Add Intel copyright
 */

#include <AVSCommon/Utils/Logger/Logger.h>

#include "SampleApp/StopPortAudioStreamObserver.h"

namespace alexaClientSDK {
namespace sampleApp {

// Logging tag
static const std::string TAG("StopPortAudioStreamObserver");

/**
 * Create a LogEntry using this file's TAG and the specified event string.
 *
 * @param The event string for this @c LogEntry.
 */
#define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)

std::shared_ptr<StopPortAudioStreamObserver> StopPortAudioStreamObserver::create(
        std::shared_ptr<PortAudioMicrophoneWrapper> micWrapper)
{
    // Empty mic wrapper
    if(!micWrapper) {
        ACSDK_ERROR(LX("createFailed").d("reason", "nullMicWrapper"));
        return nullptr;
    }
    return std::make_shared<StopPortAudioStreamObserver>(
            StopPortAudioStreamObserver(micWrapper));
}

void StopPortAudioStreamObserver::onDialogUXStateChanged(DialogUXState newState) {
    if(newState == DialogUXState::FINISHED || newState == DialogUXState::IDLE) {
        if(m_micWrapper->isStreaming()) {
            if(!m_micWrapper->stopStreamingMicrophoneData()) {
                ACSDK_ERROR(LX("onDialogUXStateChanged").d("reason", "stopStreamFailed"));
            }
        }
    }
}

StopPortAudioStreamObserver::~StopPortAudioStreamObserver() {}

StopPortAudioStreamObserver::StopPortAudioStreamObserver(
        std::shared_ptr<PortAudioMicrophoneWrapper> micWrapper) :
    m_micWrapper(micWrapper)
{}

} // sampleApp
} // alexaClientSDK
