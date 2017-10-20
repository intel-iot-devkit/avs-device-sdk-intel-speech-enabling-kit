/**
 * ResetAppTimer.h
 *
 * TODO: Add Intel copyright
 */

#ifndef ALEXA_CLIENT_SDK_SAMPLE_APP_INCLUDE_SAMPLE_APP_RESET_APP_TIMER_H_
#define ALEXA_CLIENT_SDK_SAMPLE_APP_INCLUDE_SAMPLE_APP_RESET_APP_TIMER_H_

#include <chrono>
#include <atomic>
#include <memory>
#include <mutex>
#include <condition_variable>

#include <DefaultClient/DefaultClient.h>
#include <AVSCommon/SDKInterfaces/DialogUXStateObserverInterface.h>

namespace alexaClientSDK {
namespace sampleApp {

using namespace defaultClient;
using namespace avsCommon::sdkInterfaces;

class ResetAppTimer : public DialogUXStateObserverInterface {
public:
    /**
     * Creates a shared pointer to a @c ResetAppTimer.
     *
     * @param client @c DefaultClient object for the app
     * @param timeout Milliseconds to wait until resetting the app
     * @return @c ResetAppTimer if successful, otherwise @c nullptr
     */
    static std::shared_ptr<ResetAppTimer> create(
            std::shared_ptr<DefaultClient> client,
            std::chrono::milliseconds timeout);

    /// Dialog state change callback override
    void onDialogUXStateChanged(DialogUXState state) override;
    /**
     * Destructor.
     */
    ~ResetAppTimer() = default;

private:
    /**
     * Constructor
     *
     * @param client @c DefaultClient object for the app
     * @param timeout Milliseconds to wait until resetting the app
     */
    ResetAppTimer(
            std::shared_ptr<DefaultClient> client,
            std::chrono::milliseconds timeout);

    /**
     * Start the timer.
     *
     * \note The timer will not be started if it is already running.
     *
     * @return @c true if the timer was started, @c false if it was not started
     */
    bool startTimer();
    
    /**
     * Stop the timer.
     */
    void stopTimer();

    /// Asyncronous run method for doing the count down
    void run();

    /// App client
    std::shared_ptr<DefaultClient> m_client;

    /// Timeout to wait when the timer is started
    std::chrono::milliseconds m_timeout;

    /// Flag for if the timer is currently running
    std::atomic<bool> m_running;

    /// Mutex used for waiting on the condition variable
    std::mutex m_mut;

    /// Conditional variable for executing the sleep
    std::condition_variable m_cv;
};

} // sampleApp
} // alexaClientSDK

#endif // ALEXA_CLIENT_SDK_SAMPLE_APP_INCLUDE_SAMPLE_APP_RESET_APP_TIMER_H_
