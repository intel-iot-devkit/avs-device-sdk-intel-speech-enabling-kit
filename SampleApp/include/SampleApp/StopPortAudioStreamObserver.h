/**
 * StopPortAudioStreamObserver.h
 *
 * TODO: Add Intel copyright
 */

#ifndef ALEXA_CLIENT_SDK_SAMPLE_APP_INCLUDE_SAMPLE_APP_STOP_PA_OBSERVER_H_
#define ALEXA_CLIENT_SDK_SAMPLE_APP_INCLUDE_SAMPLE_APP_STOP_PA_OBSERVER_H_

#include <memory>

#include <AVSCommon/SDKInterfaces/DialogUXStateObserverInterface.h>
#include <AVSCommon/AVS/AudioInputStream.h>

#include "SampleApp/PortAudioMicrophoneWrapper.h"

namespace alexaClientSDK {
namespace sampleApp {

using namespace avsCommon::sdkInterfaces;
using namespace avsCommon::avs;

/**
 * Class which implements the @c DialogObserverInterface to observe the changes
 * in the state in relation to the cloud, so that when the FINISHED state is
 * recheached the PortAudio audio stream can be stopped until needed again.
 */
class StopPortAudioStreamObserver : public DialogUXStateObserverInterface {
public:
    /**
     * Creates a new pointer to a @c StopPortAudioStreamObserver.
     *
     * @param micWrapper Handle to the @c PortAudioWrapper
     * @return @c StopPortAudioStreamObserver, nullptr otherwise
     */
    static std::shared_ptr<StopPortAudioStreamObserver> create(
        std::shared_ptr<PortAudioMicrophoneWrapper> micWrapper);
    
    /**
     * Callback for when the UX state changes.
     *
     * @param newState - The new state of the dialog
     */
    void onDialogUXStateChanged(DialogUXState newState);

    /**
     * Destructor.
     */
    ~StopPortAudioStreamObserver();

private:

    /**
     * Constructor.
     *
     * @param micWrapper Handle to the @c PortAudioWrapper
     */
    StopPortAudioStreamObserver(
        std::shared_ptr<PortAudioMicrophoneWrapper> micWrapper);

    /// Handle to the port audio wrapper to be able to stop the audio stream
    std::shared_ptr<PortAudioMicrophoneWrapper> m_micWrapper;
};

} // sampleApp
} // alexaClientSDK

#endif // ALEXA_CLIENT_SDK_SAMPLE_APP_INCLUDE_SAMPLE_APP_STOP_PA_OBSERVER_H_
