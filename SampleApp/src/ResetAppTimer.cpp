/**
 * ResetAppTimer.cpp
 *
 * TODO: Add Intel copyright
 */

#include <thread>

#include "SampleApp/ResetAppTimer.h"

namespace alexaClientSDK {
namespace sampleApp {

// Logging tag
static const std::string TAG("SocketHardwareController");
#define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)

std::shared_ptr<ResetAppTimer> ResetAppTimer::create(
        std::shared_ptr<DefaultClient> client,
        std::chrono::milliseconds timeout)
{
    return std::shared_ptr<ResetAppTimer>(new ResetAppTimer(client, timeout));
}

ResetAppTimer::ResetAppTimer(
        std::shared_ptr<DefaultClient> client,
        std::chrono::milliseconds timeout) :
    m_client(client), m_timeout(timeout), m_running(false)
{}

void ResetAppTimer::onDialogUXStateChanged(DialogUXState state) {
    if(DialogUXState::LISTENING == state) {
        if(!startTimer()) {
            ACSDK_ERROR(LX("startTimerFailed").d("reason", "timerAlreadyRunning"));
        }
    } else if(m_running) {
        stopTimer();
    }
}

bool ResetAppTimer::startTimer() {
    if(m_running) return false;
    auto th = std::thread(&ResetAppTimer::run, this);
    th.detach();
    return true;
}

void ResetAppTimer::stopTimer() {
    if(!m_running) return;
    m_cv.notify_all();
}

void ResetAppTimer::run() {
    std::unique_lock<std::mutex> lk(m_mut);
    auto now = std::chrono::system_clock::now();
    if(m_cv.wait_until(lk, now + m_timeout) == std::cv_status::timeout) {
        m_client->stopForegroundActivity();
    }
    m_running = false;
}

} // sampleApp
} // alexaClientSDK
