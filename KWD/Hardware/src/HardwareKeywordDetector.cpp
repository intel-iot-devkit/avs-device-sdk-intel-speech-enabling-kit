/**
 * HardwareKeyWordDetector.cpp
 *
 * TODO: Add Intel copywrite
 */

#include "Hardware/HardwareKeywordDetector.h"

#include <AVSCommon/Utils/Logger/Logger.h>

#define SAMPLE_OFFSET 9000

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
                stream, audioFormat, controller, keyWordObservers,
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
    AudioFormat audioFormat,
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
    std::unique_ptr<KeywordDetection> detection;
    int streamIdx = 0;

    // Notify the state observers, abd set to ACTIVE
    notifyKeyWordDetectorStateObservers(
        KeyWordDetectorStateObserverInterface::KeyWordDetectorState::ACTIVE);

    while(!m_isShuttingDown) {
        detection = m_controller->read(m_timeout);

        // If detection if @c nullptr, then a timeout occurred
        if(!detection) {
            continue;
        }

        // Advance the reader to where the writer currently is
        m_streamReader->seek(0, AudioInputStream::Reader::Reference::BEFORE_WRITER);
        // Get the current index of the reader, which should be at the end
        streamIdx = m_streamReader->tell();

        int offset = (streamIdx == 0) ? 0 : SAMPLE_OFFSET;
        auto begin = streamIdx + detection->getBegin() - offset;
        auto end = streamIdx + detection->getEnd() - offset;

        ACSDK_DEBUG(LX("detectionLoop")
                .d("event", "keywordDetection")
                .d("sds_offset", streamIdx)
                .d("begin", begin)
                .d("end", end));

        notifyKeyWordObservers(
                m_stream, detection->getKeyword(),
                begin, end);
    }
}

} // kwd
} // alexaClientSdk
