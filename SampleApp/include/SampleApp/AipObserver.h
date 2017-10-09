/**
 * AipObserver.h
 *
 * TODO: Add Intel copyright
 */

#ifndef ALEXA_CLIENT_SDK_SAMPLE_APP_INCLUDE_SAMPLE_APP_AIP_OBSERVER_H_
#define ALEXA_CLIENT_SDK_SAMPLE_APP_INCLUDE_SAMPLE_APP_AIP_OBSERVER_H_

#include <AVSCommon/SDKInterfaces/AudioInputProcessorObserverInterface.h>

namespace alexaClientSDK {
namespace sampleApp {

using namespace avsCommon::sdkInterfaces;

typedef AudioInputProcessorObserverInterface::State State;

class AipObserver : public AudioInputProcessorObserverInterface {
public:

    /**
     * Creates a new pointer to a AipObserver.
     *
     * @return @c AipObserver, or @c nullptr if an error occurrs
     */
    static std::shared_ptr<AipObserver> create();

    /**
     * Callback
     */
    void onStateChanged(State state) override;

    /**
     * Destructor.
     */
    ~AipObserver() = default;

private:
    /**
     * Constructor.
     */
    AipObserver();
};

} // sampleApp
} // alexaClientSDK

#endif // ALEXA_CLIENT_SDK_SAMPLE_APP_INCLUDE_SAMPLE_APP_AIP_OBSERVER_H_
