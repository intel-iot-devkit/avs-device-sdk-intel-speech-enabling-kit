/**
 * StartPortAudioStreamObserver.h
 *
 * TODO: Add Intel copyright
 */

#ifndef ALEXA_CLIENT_SDK_SAMPLE_APP_INCLUDE_SAMPLE_APP_KW_OBSERVER_H_
#define ALEXA_CLIENT_SDK_SAMPLE_APP_INCLUDE_SAMPLE_APP_KW_OBSERVER_H_

#include <memory>

#include <AVSCommon/SDKInterfaces/KeyWordObserverInterface.h>
#include <AVSCommon/AVS/AudioInputStream.h>

#include "SampleApp/PortAudioMicrophoneWrapper.h"

namespace alexaClientSDK {
namespace sampleApp {
    
using namespace avsCommon::avs;
using namespace avsCommon::sdkInterfaces;

/**
 * Keyword observer to start the PA stream on a notification from the 
 */
class StartPortAudioStreamObserver : public KeyWordObserverInterface {
public:
    /**
     * Creates a new pointer to a @c StartPortAudioStreamObserver.
     *
     * @param micWrapper Handle to the @c PortAudioWrapper
     * @return @c StartPortAudioStreamObserver, nullptr otherwise
     */
    static std::shared_ptr<StartPortAudioStreamObserver> create(
        std::shared_ptr<PortAudioMicrophoneWrapper> micWrapper);

    /**
     * Callback for when a keyword is detected.
     */
    void onKeyWordDetected(
            std::shared_ptr<AudioInputStream> stream,
            std::string keyword,
            AudioInputStream::Index begin = KeyWordObserverInterface::UNSPECIFIED_INDEX,
            AudioInputStream::Index end = KeyWordObserverInterface::UNSPECIFIED_INDEX);

    /**
     * Destructor.
     */
    ~StartPortAudioStreamObserver() override;

private:
    /**
     * Constructor.
     *
     * @param micWrapper Handle to the @c PortAudioWrapper
     */
    StartPortAudioStreamObserver(
        std::shared_ptr<PortAudioMicrophoneWrapper> micWrapper);
    
    /// Handle to the port audio wrapper to be able to start the audio stream
    std::shared_ptr<PortAudioMicrophoneWrapper> m_micWrapper;
};

}
} // alexaClientSdk

#endif // ALEXA_CLIENT_SDK_SAMPLE_APP_INCLUDE_SAMPLE_APP_KW_OBSERVER_H_
