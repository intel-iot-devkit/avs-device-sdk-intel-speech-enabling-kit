/**
 * StopPortAudioStreamObserver.cpp
 *
 * TODO: Add Intel copyright
 */

#include <AVSCommon/Utils/Logger/Logger.h>
#include <AVSCommon/Utils/TestLogger/TestLogger.h>

#include "SampleApp/StopPortAudioStreamObserver.h"
namespace alexaClientSDK {
namespace sampleApp {

using namespace avsCommon::utils::testLogger;

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
    Log::info(TAG.c_str(), "Dialog State Changed: %s", stateToString(newState).c_str());
    if(newState == DialogUXState::FINISHED || newState == DialogUXState::IDLE) {
        if(m_micWrapper->isStreaming()) {
            if(!m_micWrapper->stopStreamingMicrophoneData()) {
                ACSDK_ERROR(LX("onDialogUXStateChanged").d("reason", "stopStreamFailed"));
            } /*else {
                m_micWrapper->startStreamingMicrophoneData();
            }*/
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
