/**
 * HardwareKeyWordDetector.cpp
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

std::unique_ptr<HardwareKeywordDetector> HardwareKeywordDetector::create(
    std::shared_ptr<AudioInputStream> stream,
    AudioFormat audioFormat,
    std::shared_ptr<AbstractHardwareController> controller,
    SetKeyWordObserverInterface keyWordObservers,
    SetKeyWordDetectorStateObservers keyWordDetectorStateObservers,
    std::chrono::milliseconds timeout)
{
    // Verify that the given stream is not NULL
    if (!stream) {
        ACSDK_ERROR(LX("createFailed").d("reason", "nullStream"));
        return nullptr;
    }

    // Verify that the given audio stream is not NULL
    if(!controller) {
        ACSDK_ERROR(LX("createFailed").d("reason", "nullHwController"));
        return nullptr;
    }

    // TODO: ACSDK-249 - Investigate cpu usage of converting bytes between 
    // endianness and if it's not too much, do it.
    if (isByteswappingRequired(audioFormat)) {
        ACSDK_ERROR(LX("createFailed").d("reason", "endianMismatch"));
        return nullptr;
    }

    // Initialize the detector, and return it
    std::unique_ptr<HardwareKeywordDetector> detector(
            new HardwareKeywordDetector(
                stream, controller, keyWordObservers,
                keyWordDetectorStateObservers, 
                timeout));

    if(!detector->init()) {
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
    std::shared_ptr<AbstractHardwareController> controller,
    SetKeyWordObserverInterface keyWordObservers,
    SetKeyWordDetectorStateObservers keyWordDetectorStateObservers,
    std::chrono::milliseconds timeout) :
        AbstractKeywordDetector(keyWordObservers, keyWordDetectorStateObservers),
        m_stream{stream}, m_controller{controller}, m_timeout(timeout)
{ }

bool HardwareKeywordDetector::init() {
    m_streamReader = m_stream->createReader(AudioInputStream::Reader::Policy::BLOCKING);

    if (!m_streamReader) {
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
    std::unique_ptr<KeywordDetection> detection;
    
    while(!m_isShuttingDown) {
        detection = m_controller->read(m_timeout);

        // If detection if @c nullptr, then a timeout occurred
        if(!detection) {
            continue;
        }

        if(detection->getBegin() < 0 && detection->getEnd() < 0) {
            notifyKeyWordObservers(
                    m_stream, detection->getKeyword(),
                    KeyWordObserverInterface::UNSPECIFIED_INDEX,
                    m_streamReader->tell());
        } else {
            notifyKeyWordObservers(
                    m_stream, detection->getKeyword(),
                    detection->getBegin(),
                    detection->getEnd());
        }
    }

    m_streamReader->close();
}

} // kwd
} // alexaClientSdk
