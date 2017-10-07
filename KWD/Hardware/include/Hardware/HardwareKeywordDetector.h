/**
 * HardwareKeywordDetector.h
 * 
 * @author Kevin Midkiff (kevin.midkiff@intel.com)
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
     * @return A new @c HardwareKeywordDetector, or @c nullptr if the operation 
     * failed
     */
    static std::unique_ptr<HardwareKeywordDetector> create(
        std::shared_ptr<AudioInputStream> stream,
        avsCommon::utils::AudioFormat audioFormat,
        SetKeyWordObserverInterface keyWordObservers,
        SetKeyWordDetectorStateObservers keyWordDetectorStateObservers,
        std::chrono::milliseconds msToPushPerIteration = std::chrono::milliseconds(20));
    
    /**
     * Destructor.
     */
    ~HardwareKeywordDetector() override;
private:
    /**
     * Constructor.
     *
     * @param stream The stream of audio data. This should be formatted in LPCM
     * encoded with 16 bits per sample and have a sample rate of 16 kHz. 
     * Additionally, the data should be in little endian format.
     * @param audioFormat The format of the audio data located within the stream.
     * @param keyWordObservers The observers to notify of keyword detections.
     * @param keyWordDetectorStateObservers The observers to notify of state 
     * changes in the engine.
     * @return A new @c HardwareKeywordDetector, or @c nullptr if the operation failed
     */
    HardwareKeywordDetector(
        std::shared_ptr<AudioInputStream> stream,
        avsCommon::utils::AudioFormat audioFormat,
        SetKeyWordObserverInterface keyWordObservers,
        SetKeyWordDetectorStateObservers keyWordDetectorStateObservers,
        std::chrono::milliseconds msToPushPerIteration);

    /**
     * Initializes the stream reader and kicks off a thread to read data from
     * the stream. This method should only be called once with each new @c
     * HardwareKeywordDetector.
     *
     * @param audioFormat The format of the audio data located within the stream
     * @return @c true if the audio format is compatible, and @c false otherwise 
     */
    bool init(AudioFormat audioFormat);

    /// The main function that reads data off of the audio stream
    void detectionLoop();

    /// Indicates whether the internal main loop should keep running.
    std::atomic<bool> m_isShuttingDown;

    /// The stream of audio data.
    const std::shared_ptr<avsCommon::avs::AudioInputStream> m_stream;

    /// The reader that will be used to read audio data from the stream.
    std::shared_ptr<avsCommon::avs::AudioInputStream::Reader> m_streamReader;

    /**
     * Internal thread that waits for a signal from the hardware that the
     * keyword has been detected.
     */
    std::thread m_detectionThread;

    /**
     * The max number of samples to push into the underlying engine per 
     * iteration. This will be determined based on the sampling rate of the 
     * audio data passed in.
     */
    // TODO: May not need this
    const size_t m_maxSamplesPerPush;
};

} // namespace kwd
} // namespace alexaClientSdk

#endif // ALEXA_CLIENT_SDK_KWD_HARDWARE_INCLUDE_KWD_HARDWARE_KEY_WORD_DETECTOR_H_

