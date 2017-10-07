/**
 * HardwareKeyWordDetector.cpp
 * 
 * @author Kevin Midkiff (kevin.midkiff@intel.com)
 *
 * TODO: Add Intel copywrite
 */

#include "Hardware/HardwareKeywordDetector.h"

#include <AVSCommon/Utils/Logger/Logger.h>

namespace alexaClientSDK {
namespace kwd {

using namespace avsCommon;
using namespace avsCommon::avs;
using namespace avsCommon::sdkInterfaces;
using namespace avsCommon::utils;

// Logging tag
static const std::string TAG("HardwareKeywordDetector");

/**
 * Create a LogEntry using this file's TAG and the specified event string.
 *
 * @param The event string for this @c LogEntry.
 */
#define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)

/// The number of hertz per kilohertz.
static const size_t HERTZ_PER_KILOHERTZ = 1000;

/// The timeout to use for read calls to the SharedDataStream.
const std::chrono::milliseconds TIMEOUT_FOR_READ_CALLS = std::chrono::milliseconds(1000);

std::unique_ptr<HardwareKeywordDetector> HardwareKeywordDetector::create(
    std::shared_ptr<AudioInputStream> stream,
    avsCommon::utils::AudioFormat audioFormat,
    SetKeyWordObserverInterface keyWordObservers,
    SetKeyWordDetectorStateObservers keyWordDetectorStateObservers,
    std::chrono::milliseconds msToPushPerIteration)
{
    // Verify that the given audio stream is not NULL
    if(!stream) {
        ACSDK_ERROR(LX("createFailed").d("reason", "nullStream"));
        return nullptr;
    }


    // TODO: ACSDK-249 - Investigate cpu usage of converting bytes between 
    // endianness and if it's not too much, do it.
    if(isByteswappingRequired(audioFormat)) {
        ACSDK_ERROR(LX("createFailed").d("reason", "endianMismatch"));
        return nullptr;
    }
    
    // Initialize the detector, and return it
    std::unique_ptr<HardwareKeywordDetector> detector(
            new HardwareKeywordDetector(
                stream, audioFormat, keyWordObservers,
                keyWordDetectorStateObservers, msToPushPerIteration));
    if (!detector->init(audioFormat)) {
        ACSDK_ERROR(LX("createFailed").d("reason", "initDetectorFailed"));
        return nullptr;
    }
    return detector;
}

HardwareKeywordDetector::~HardwareKeywordDetector() {
    m_isShuttingDown = true;
    if(m_detectionThread.joinable()) {
        m_detectionThread.join();
    }
}


HardwareKeywordDetector::HardwareKeywordDetector(
    std::shared_ptr<AudioInputStream> stream,
    avsCommon::utils::AudioFormat audioFormat,
    SetKeyWordObserverInterface keyWordObservers,
    SetKeyWordDetectorStateObservers keyWordDetectorStateObservers,
    std::chrono::milliseconds msToPushPerIteration) :
        AbstractKeywordDetector(keyWordObservers, keyWordDetectorStateObservers),
        m_stream{stream},
        m_maxSamplesPerPush{(audioFormat.sampleRateHz / HERTZ_PER_KILOHERTZ) * msToPushPerIteration.count()}
{ }

bool HardwareKeywordDetector::init(AudioFormat audioFormat) {
    // Get a stream reader from the given audio stream
    m_streamReader = m_stream->createReader(AudioInputStream::Reader::Policy::BLOCKING);
    if(!m_streamReader) {
        ACSDK_ERROR(LX("initFailed").d("reason", "createStreamReaderFailed"));
        return false;
    }

    m_isShuttingDown = false;
    m_detectionThread = std::thread(&HardwareKeywordDetector::detectionLoop, this);
    return true;
    
}

void HardwareKeywordDetector::detectionLoop() {
    // Notify the state observers, abd set to ACTIVE
    notifyKeyWordDetectorStateObservers(
        KeyWordDetectorStateObserverInterface::KeyWordDetectorState::ACTIVE);
    // int16_t audioDataToPush[m_maxSamplesPerPush];
    // ssize_t wordsRead;
    
    while(!m_isShuttingDown) {
        // TODO: Read from stream and parse
    }

    m_streamReader->close();
}

} // kwd
} // alexaClientSdk
