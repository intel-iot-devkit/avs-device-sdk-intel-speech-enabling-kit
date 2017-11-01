/**
 * PortAudioObserver.h
 *
 * TODO: Add Intel copyright
 */

#ifndef ALEXA_CLIENT_SDK_SAMPLE_APP_INCLUDE_SAMPLE_APP_PA_OBSERVER_H_
#define ALEXA_CLIENT_SDK_SAMPLE_APP_INCLUDE_SAMPLE_APP_PA_OBSERVER_H_

#include <memory>

#include <AVSCommon/SDKInterfaces/KeyWordObserverInterface.h>
#include <AVSCommon/SDKInterfaces/DialogUXStateObserverInterface.h>
#include <AVSCommon/SDKInterfaces/AudioInputProcessorObserverInterface.h>
#include <AVSCommon/AVS/AudioInputStream.h>

#include "SampleApp/PortAudioMicrophoneWrapper.h"

namespace alexaClientSDK {
namespace sampleApp {
    
using namespace avsCommon::avs;
using namespace avsCommon::sdkInterfaces;

/**
 * Observer which interacts with PortAudio based on keyword detections, and the
 * changing of the dialog state.
 *
 * IMPORTANT: This observer must be added to the @c AudioInputProcessor observers
 * AFTER the @c AlsaHardwareController is added, so that the correct sequence of
 * events for opening the microphone occur.
 */
class PortAudioObserver 
    : public KeyWordObserverInterface
    , public DialogUXStateObserverInterface
    , public AudioInputProcessorObserverInterface {
public:
    /// Alias to the @c AudioInputProcessorObserverInterface::state for brevity
    using AipState = AudioInputProcessorObserverInterface::State;

    /**
     * Creates a new pointer to a @c StartPortAudioStreamObserver.
     *
     * @param micWrapper Handle to the @c PortAudioWrapper
     * @return @c StartPortAudioStreamObserver, nullptr otherwise
     */
    static std::shared_ptr<PortAudioObserver> create(
        std::shared_ptr<PortAudioMicrophoneWrapper> micWrapper);

    /// @name KeyWordObserverInterface Functions
    /// @{
    /**
     * Callback for when a keyword is detected.
     */
    void onKeyWordDetected(
            std::shared_ptr<AudioInputStream> stream,
            std::string keyword,
            AudioInputStream::Index begin = KeyWordObserverInterface::UNSPECIFIED_INDEX,
            AudioInputStream::Index end = KeyWordObserverInterface::UNSPECIFIED_INDEX) override;
    /// @}

    /// @name DialogUXStateObserverInterface Functions
    /// @{
    /**
     * Callback for when the UX state changes.
     *
     * @param newState - The new state of the dialog
     */
    void onDialogUXStateChanged(DialogUXState newState) override;
    /// @}
    
    /// @name AudioInputProcessorObserverInterface Functions
    /// @{
    /**
     * Callback for when @c AudioInputProcessor state changes.
     *
     * @param state - The new state of the @c AudioInputProcessor
     */
    void onStateChanged(AipState state) override;
    /// @}

    /**
     * Destructor.
     */
    ~PortAudioObserver() override;

private:
    /**
     * Constructor.
     *
     * @param micWrapper Handle to the @c PortAudioWrapper
     */
    PortAudioObserver(
        std::shared_ptr<PortAudioMicrophoneWrapper> micWrapper);
    
    /// Handle to the port audio wrapper to be able to start the audio stream
    std::shared_ptr<PortAudioMicrophoneWrapper> m_micWrapper;
};

}
} // alexaClientSdk

#endif // ALEXA_CLIENT_SDK_SAMPLE_APP_INCLUDE_SAMPLE_APP_PA_OBSERVER_H_
