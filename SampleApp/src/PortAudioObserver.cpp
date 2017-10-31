/**
 * PortAudioObserver.cpp
 *
 * TODO: Add Intel copyright
 */

#include <AVSCommon/Utils/Logger/Logger.h>

#include "SampleApp/PortAudioObserver.h"

namespace alexaClientSDK {
namespace sampleApp {

// Logging tag
static const std::string TAG("PortAudioObserver");

/**
 * Create a LogEntry using this file's TAG and the specified event string.
 *
 * @param The event string for this @c LogEntry.
 */
#define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)

std::shared_ptr<PortAudioObserver> PortAudioObserver::create(
        std::shared_ptr<PortAudioMicrophoneWrapper> micWrapper)
{
    // Empty mic wrapper
    if(!micWrapper) {
        ACSDK_ERROR(LX("createFailed").d("reason", "nullMicWrapper"));
        return nullptr;
    }
    return std::make_shared<PortAudioObserver>(
            PortAudioObserver(micWrapper));
}

void PortAudioObserver::onKeyWordDetected(
        std::shared_ptr<AudioInputStream> stream,
        std::string keyword,
        AudioInputStream::Index begin,
        AudioInputStream::Index end)
{
    if(!m_micWrapper->isStreaming()) {
        ACSDK_DEBUG(LX("onKeyWordDetected")
                .d("event", "starting microphone stream"));
        if(!m_micWrapper->startStreamingMicrophoneData()) {
            ACSDK_ERROR(LX("onKeyWordDetectedFailed")
                    .d("reason", "startStreamFailed"));
        }
    }
}

void PortAudioObserver::onDialogUXStateChanged(DialogUXState newState) {
    if((newState == DialogUXState::THINKING || newState == DialogUXState::IDLE) 
            && m_micWrapper->isStreaming()) {
        // If the we the dialog has reached either the THINKING or IDLE state and
        // the microphone is streaming audio, then stop the audio stream.
        ACSDK_DEBUG(LX("onDialogUXStateChanged")
                .d("event", "starting microphone stream"));
        if(!m_micWrapper->stopStreamingMicrophoneData()) {
            ACSDK_ERROR(LX("onDialogUXStateChanged")
                    .d("reason", "stopStreamFailed"));
        }
    } else if(newState == DialogUXState::LISTENING && !m_micWrapper->isStreaming()) {
        // If the dialog has reached the LISTENING state and the microphone is
        // not currently streaming audio, then we have reached the EXPECTING_SPEECH
        // state, and should re-enable the audio streaming from the microphone
        ACSDK_DEBUG(LX("onDialogUXStateChanged")
                .d("event", "EXPECTING_SPEECH"));
        if(!m_micWrapper->startStreamingMicrophoneData()) {
            ACSDK_ERROR(LX("onDialogUXStateChanged")
                    .d("reason", "startStreamFailed"));
        }
    }
}

PortAudioObserver::~PortAudioObserver() {}

PortAudioObserver::PortAudioObserver(
        std::shared_ptr<PortAudioMicrophoneWrapper> micWrapper) :
    m_micWrapper(micWrapper)
{}

} // sampleApp
} // alexaClientSDK
