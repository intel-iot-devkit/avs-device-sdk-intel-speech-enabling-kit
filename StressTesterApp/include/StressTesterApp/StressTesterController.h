/**
 * StressTesterController.h
 *
 * TODO: Add Intel copyright
 */

#ifndef ALEXA_CLIENT_SDK_STRESS_TESTER_INCLUDE_CONTROLLER_
#define ALEXA_CLIENT_SDK_STRESS_TESTER_INCLUDE_CONTROLLER_

#include <memory>
#include <chrono>
#include <string>

#include <AVSCommon/SDKInterfaces/DialogUXStateObserverInterface.h>
#include <AVSCommon/AVS/AudioInputStream.h>
#include <HardwareController/AbstractHardwareController.h>

namespace alexaClientSDK {
namespace stressTesterApp {

using namespace avsCommon::sdkInterfaces;
using namespace avsCommon::avs;
using namespace kwd;

class StressTesterController : public DialogUXStateObserverInterface, 
                               public AbstractHardwareController {
public:
    static std::shared_ptr<StressTesterController> create(
            std::chrono::milliseconds interval, std::string keyword);

    /**
     * Callback for when the UX state changes.
     *
     * @param newState - The new state of the dialog
     */
    void onDialogUXStateChanged(DialogUXState newState);


    /**
     * Read a @c KeywordDetection from the hardware controller.
     *
     * @param timeout Timeout for the read
     * @return @c KeywordDetection when a detection occurs, otherwise @c nullptr
     * if an error occurs, or a timeout
     */
    std::unique_ptr<KeywordDetection> read(std::chrono::milliseconds timeout);

    /**
     * Destructor.
     */
    ~StressTesterController();

private:
    /**
     * Constructor.
     *
     * @param interval Time to wait to trigger another read.
     * @param keyword Keyword to say was recognized
     */
    StressTesterController(std::chrono::milliseconds interval, std::string keyword);

    /**
     * Method to run asyncronously to trigger the ability to do a read.
     */
    void triggerRead();

    /// Interval to trigger another read
    std::chrono::milliseconds m_interval;

    /// Keyword which was recognized
    std::string m_keyword;
};

} // stressTesterApp
} // alexaClientSDK

#endif // ALEXA_CLIENT_SDK_STRESS_TESTER_INCLUDE_CONTROLLER_
