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
    return std::shared_ptr<PortAudioObserver>(new PortAudioObserver(micWrapper));
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
                .d("event", "Stopping microphone stream"));
        if(!m_micWrapper->stopStreamingMicrophoneData()) {
            ACSDK_ERROR(LX("onDialogUXStateChanged")
                    .d("reason", "stopStreamFailed"));
        }
    } else if(newState == DialogUXState::LISTENING && m_isExpectingSpeech 
            && !m_micWrapper->isStreaming()) {
        // If we have reached the LISTENING state and we know that we are
        // supposed to be expecting speech then the mic needs to be opened
        ACSDK_DEBUG(LX("onDialogUXStateChanged")
                .d("event", "LISTENING && EXPECTING_SPEECH"));
        if(!m_micWrapper->startStreamingMicrophoneData()) {
            ACSDK_ERROR(LX("onDialogUXStateChanged")
                    .d("reason", "startStreamFailed"));
        }
    }
}

void PortAudioObserver::onStateChanged(AipState state) {
    if(state == AipState::EXPECTING_SPEECH) {
        // Set the expecting speech flag
        m_isExpectingSpeech = true;
    } else if(state == AipState::IDLE) {
        // If we were previously expecting speech, then set the flag to no
        // longer expect it
        if(m_isExpectingSpeech)
            m_isExpectingSpeech = false;
    }
}

PortAudioObserver::~PortAudioObserver() {}

PortAudioObserver::PortAudioObserver(
        std::shared_ptr<PortAudioMicrophoneWrapper> micWrapper) :
    m_micWrapper(micWrapper), m_isExpectingSpeech(false)
{}

} // sampleApp
} // alexaClientSDK
