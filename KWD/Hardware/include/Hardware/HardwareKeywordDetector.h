/**
 * HardwareKeywordDetector.h
 *
 * TODO: Add Intel copywrite
 *
 */

#ifndef ALEXA_CLIENT_SDK_KWD_HARDWARE_INCLUDE_KWD_HARDWARE_KEY_WORD_DETECTOR_H_
#define ALEXA_CLIENT_SDK_KWD_HARDWARE_INCLUDE_KWD_HARDWARE_KEY_WORD_DETECTOR_H_

#include <atomic>
#include <string>
#include <thread>

#include <AVSCommon/Utils/AudioFormat.h>
#include <AVSCommon/AVS/AudioInputStream.h>
#include <AVSCommon/SDKInterfaces/KeyWordObserverInterface.h>
#include <AVSCommon/SDKInterfaces/KeyWordDetectorStateObserverInterface.h>

#include "KWD/AbstractKeywordDetector.h"
#include "Hardware/KeywordDetection.h"
#include "HardwareController/AbstractHardwareController.h"

namespace alexaClientSDK {
namespace kwd {

using namespace avsCommon;
using namespace avsCommon::avs;
using namespace avsCommon::sdkInterfaces;
using namespace avsCommon::utils;

/**
 * Type that encapsulates an unordered set of @c 
 * KeyWordDetectorObserverInterface objects
 */
typedef std::unordered_set<std::shared_ptr<KeyWordObserverInterface>> SetKeyWordObserverInterface;

/**
 * Type that encapsulates an unordered set of @c 
 * KeyWordDetectorStateObserverInterface objects
 */
typedef std::unordered_set<std::shared_ptr<KeyWordDetectorStateObserverInterface>> SetKeyWordDetectorStateObservers;

class HardwareKeywordDetector : public AbstractKeywordDetector {
public:

    /**
     * Creates a @c HardwareKeywordDetector.
     *
     * @param stream The stream of audio data. This should be formatted in 
     * LPCM encoded with 16 bits per sample and have a sample rate of 16 kHz. 
     * Additionally, the data should be in little endian format.
     * @param audioFormat The format of the audio data located within the stream.
     * @param controller Handle to the hardware controller for receiving events
     * for when the keyword has been detected
     * @param keyWordObservers The observers to notify of keyword detections.
     * @param keyWordDetectorStateObservers The observers to notify of state 
     * changes in the engine.
     * @param timeout Timeout for checking if the thread needs to stop
     * @return A new @c HardwareKeywordDetector, or @c nullptr if the operation 
     * failed
     */
    static std::unique_ptr<HardwareKeywordDetector> create(
        std::shared_ptr<AudioInputStream> stream,
        AudioFormat audioFormat,
        std::shared_ptr<AbstractHardwareController> controller,
        SetKeyWordObserverInterface keyWordObservers,
        SetKeyWordDetectorStateObservers keyWordDetectorStateObservers,
        std::chrono::milliseconds timeout = std::chrono::milliseconds(250),
        std::chrono::milliseconds msToPushPerIteration = std::chrono::milliseconds(20));
    
    /**
     * Destructor.
     */
    ~HardwareKeywordDetector() override;
private:
    /**
     * Constructor.
     *
     * @param stream The stream of audio data. This should be formatted in 
     * LPCM encoded with 16 bits per sample and have a sample rate of 16 kHz. 
     * Additionally, the data should be in little endian format.
     * @param controller Handle to the hardware controller for receiving events
     * for when the keyword has been detected
     * @param keyWordObservers The observers to notify of keyword detections.
     * @param keyWordDetectorStateObservers The observers to notify of state 
     * changes in the engine.
     * @param timeout Timeout for checking if the thread needs to stop
     */
    HardwareKeywordDetector(
        std::shared_ptr<AudioInputStream> stream,
        AudioFormat audioFormat,
        std::shared_ptr<AbstractHardwareController> controller,
        SetKeyWordObserverInterface keyWordObservers,
        SetKeyWordDetectorStateObservers keyWordDetectorStateObservers,
        std::chrono::milliseconds timeout,
        std::chrono::milliseconds msToPushPerIteration);

    /**
     * Initializes the stream and kicks off a thread to poll the hardware for
     * keyword detection events.
     *
     * @return @c true if the initialization was successful, @c false otherwise.
     */
    bool init(); 

    /// The main function that polls from the hardware conntroller for kw events
    void detectionLoop();

    /**
     * The main function that reads data off of the audio stream to be able to
     * update the internal stream reader offset.
     */
    void readStreamLoop();

    /**
     * Copy of the AbstractKeywordDetector::readFromStream method which removes
     * unneeded log statement, because for the HardwareKeywordDetector and timeout
     * in reading from SDS is not a bad thing.
     */
    ssize_t readFromSds(
        std::shared_ptr<avsCommon::avs::AudioInputStream::Reader> reader,
        std::shared_ptr<avsCommon::avs::AudioInputStream> stream,
        void* buf,
        size_t nWords,
        std::chrono::milliseconds timeout,
        bool* errorOccurred);

    /// The stream of audio data.
    const std::shared_ptr<avsCommon::avs::AudioInputStream> m_stream;

    /// The reader that will be used to read audio data from the stream.
    std::shared_ptr<avsCommon::avs::AudioInputStream::Reader> m_streamReader;

    /// Handle to the hardware controller
    std::shared_ptr<AbstractHardwareController> m_controller;

    /// Timeout for polling the hardware for events
    std::chrono::milliseconds m_timeout;

    /// Indicates whether the internal main loop should keep running.
    std::atomic<bool> m_isShuttingDown;

    /// Current index of the reader in the SDS
    // std::atomic<int> m_streamIdx;
    int m_streamIdx;

    /**
     * Internal thread that waits for a signal from the hardware that the
     * keyword has been detected.
     */
    std::thread m_detectionThread;

    /**
     * Inernal thread that reads from the audio stream to update the internal
     * stream reader index.
     */
    std::thread m_readStreamThread;

    /// The max number of samples to push read from the audio stream.
    const size_t m_maxSamplesPerPush;
};

} // namespace kwd
} // namespace alexaClientSdk

#endif // ALEXA_CLIENT_SDK_KWD_HARDWARE_INCLUDE_KWD_HARDWARE_KEY_WORD_DETECTOR_H_

