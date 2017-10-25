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

/// The number of hertz per kilohertz.
static const size_t HERTZ_PER_KILOHERTZ = 1000;

/// The timeout to use for read calls to the SharedDataStream.
const std::chrono::milliseconds TIMEOUT_FOR_READ_CALLS = std::chrono::milliseconds(1000);

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
    std::chrono::milliseconds timeout,
    std::chrono::milliseconds msToPushPerIteration)
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
                timeout, msToPushPerIteration));

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
    if(m_readStreamThread.joinable()) {
        m_readStreamThread.join();
    }
}

HardwareKeywordDetector::HardwareKeywordDetector(
    std::shared_ptr<AudioInputStream> stream,
    AudioFormat audioFormat,
    std::shared_ptr<AbstractHardwareController> controller,
    SetKeyWordObserverInterface keyWordObservers,
    SetKeyWordDetectorStateObservers keyWordDetectorStateObservers,
    std::chrono::milliseconds timeout,
    std::chrono::milliseconds msToPushPerIteration) :
        AbstractKeywordDetector(keyWordObservers, keyWordDetectorStateObservers),
        m_stream{stream}, m_controller{controller}, m_timeout(timeout),
        m_streamIdx(0), m_maxSamplesPerPush{
            (audioFormat.sampleRateHz / HERTZ_PER_KILOHERTZ) * msToPushPerIteration.count()}
{ }

bool HardwareKeywordDetector::init() {
    m_streamReader = m_stream->createReader(AudioInputStream::Reader::Policy::BLOCKING);

    if (!m_streamReader) {
        ACSDK_ERROR(LX("initFailed").d("reason", "createStreamReaderFailed"));
        return false;
    }

    m_isShuttingDown = false;
    m_detectionThread = std::thread(&HardwareKeywordDetector::detectionLoop, this);
    m_readStreamThread = std::thread(&HardwareKeywordDetector::readStreamLoop, this);
    return true;
}

void HardwareKeywordDetector::readStreamLoop() {
    uint16_t audioData[m_maxSamplesPerPush];
    ssize_t wordsRead;

    while(!m_isShuttingDown) {
        bool didErrorOccur;
        wordsRead = readFromSds(
            m_streamReader, m_stream, audioData, m_maxSamplesPerPush, 
            TIMEOUT_FOR_READ_CALLS, &didErrorOccur);
        if(didErrorOccur) {
            ACSDK_ERROR(LX("detectionLoopFailed").d("reason", "readStreamFailed"));
            break;
        }
        if(wordsRead > 0) {
            m_streamIdx = m_streamReader->tell();
        }
    }

    m_streamReader->close();
}

void HardwareKeywordDetector::detectionLoop() {
    std::unique_ptr<KeywordDetection> detection;

    // Notify the state observers, abd set to ACTIVE
    notifyKeyWordDetectorStateObservers(
        KeyWordDetectorStateObserverInterface::KeyWordDetectorState::ACTIVE);

    while(!m_isShuttingDown) {
        detection = m_controller->read(m_timeout);

        // If detection if @c nullptr, then a timeout occurred
        if(!detection) {
            continue;
        }

        int offset = (m_streamIdx == 0) ? 0 : SAMPLE_OFFSET;
        auto begin = m_streamIdx + detection->getBegin() - offset;
        auto end = m_streamIdx + detection->getEnd() - offset;

        notifyKeyWordObservers(
                m_stream, detection->getKeyword(),
                begin, end);
    }
}

/// Copy of the AbstractKeywordDetector::readFromStream method which removes
/// unneeded log statement, because for the HardwareKeywordDetector and timeout
/// in reading from SDS is not a bad thing.
ssize_t HardwareKeywordDetector::readFromSds(
    std::shared_ptr<avsCommon::avs::AudioInputStream::Reader> reader,
    std::shared_ptr<avsCommon::avs::AudioInputStream> stream,
    void* buf,
    size_t nWords,
    std::chrono::milliseconds timeout,
    bool* errorOccurred) {
    if (errorOccurred) {
        *errorOccurred = false;
    }
    ssize_t wordsRead = reader->read(buf, nWords, timeout);
    // Stream has been closed
    if (wordsRead == 0) {
        ACSDK_DEBUG(LX("readFromStream").d("event", "streamClosed"));
        notifyKeyWordDetectorStateObservers(KeyWordDetectorStateObserverInterface::KeyWordDetectorState::STREAM_CLOSED);
        if (errorOccurred) {
            *errorOccurred = true;
        }
        // This represents some sort of error with the read() call
    } else if (wordsRead < 0) {
        switch (wordsRead) {
            case AudioInputStream::Reader::Error::OVERRUN:
                ACSDK_ERROR(LX("readFromStreamFailed")
                                .d("reason", "streamOverrun")
                                .d("numWordsOverrun",
                                   std::to_string(
                                       reader->tell(AudioInputStream::Reader::Reference::BEFORE_WRITER) -
                                       stream->getDataSize())));
                reader->seek(0, AudioInputStream::Reader::Reference::BEFORE_WRITER);
                break;
            case AudioInputStream::Reader::Error::TIMEDOUT:
                break;
            default:
                // We should never get this since we are using a Blocking Reader.
                ACSDK_ERROR(LX("readFromStreamFailed")
                                .d("reason", "unexpectedError")
                                // Leave as ssize_t to avoid messiness of casting to enum.
                                .d("error", wordsRead));

                notifyKeyWordDetectorStateObservers(KeyWordDetectorStateObserverInterface::KeyWordDetectorState::ERROR);
                if (errorOccurred) {
                    *errorOccurred = true;
                }
                break;
        }
    }
    return wordsRead;
}

} // kwd
} // alexaClientSdk
