/**
 * StartPortAudioStreamObserver.cpp
 *
 * TODO: Add Intel copyright
 */

#include <AVSCommon/Utils/Logger/Logger.h>

#include "SampleApp/ConsolePrinter.h"
#include "SampleApp/StartPortAudioStreamObserver.h"

namespace alexaClientSDK {
namespace sampleApp {

using namespace avsCommon::utils;

// Logging tag
static const std::string TAG("StartPortAudioStreamObserver");

/**
 * Create a LogEntry using this file's TAG and the specified event string.
 *
 * @param The event string for this @c LogEntry.
 */
#define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)

std::shared_ptr<StartPortAudioStreamObserver> StartPortAudioStreamObserver::create(
        std::shared_ptr<PortAudioMicrophoneWrapper> micWrapper)
{
    // Empty mic wrapper
    if(!micWrapper) {
        ACSDK_ERROR(LX("createFailed").d("reason", "nullMicWrapper"));
        return nullptr;
    }
    return std::make_shared<StartPortAudioStreamObserver>(
            StartPortAudioStreamObserver(micWrapper));
}

void StartPortAudioStreamObserver::onKeyWordDetected(
        std::shared_ptr<AudioInputStream> stream,
        std::string keyword,
        AudioInputStream::Index begin,
        AudioInputStream::Index end)
{
    alexaClientSDK::sampleApp::ConsolePrinter::simplePrint("Received keyword notification");
    if(!m_micWrapper->isStreaming()) {
        if(!m_micWrapper->startStreamingMicrophoneData()) {
            ACSDK_ERROR(LX("onKeyWordDetectedFailed").d("reason", "startStreamFailed"));
        }
    }
}

StartPortAudioStreamObserver::~StartPortAudioStreamObserver() {}

StartPortAudioStreamObserver::StartPortAudioStreamObserver(
        std::shared_ptr<PortAudioMicrophoneWrapper> micWrapper) :
    m_micWrapper(micWrapper)
{}

} // sampleApp
} // alexaClientSDK
