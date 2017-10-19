/**
 * StressTesterController.cpp
 *
 * TODO: Add Intel copyright
 */

#include <future>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <atomic>

#include "StressTesterApp/Logger.h"
#include "StressTesterApp/StressTesterController.h"

#define TAG "StressTesterController"

namespace alexaClientSDK {
namespace stressTesterApp {

std::shared_ptr<StressTesterController> StressTesterController::create(
        std::chrono::milliseconds interval, std::string keyword) {
    return std::shared_ptr<StressTesterController>(new StressTesterController(
                interval, keyword));
}

StressTesterController::StressTesterController(
        std::chrono::milliseconds interval, std::string keyword) :
    m_interval(interval), m_keyword(keyword), m_canRead(false),
    m_readEnabled(false)
{}

StressTesterController::~StressTesterController() {}

std::unique_ptr<KeywordDetection> StressTesterController::read(
        std::chrono::milliseconds timeout) 
{
    if(!m_canRead) {
        std::mutex mutCv;
        std::unique_lock<std::mutex> lk(mutCv);
        auto now = std::chrono::system_clock::now();

        if(m_cv.wait_until(lk, now + timeout) == std::cv_status::timeout) {
            return nullptr;
        }
    }

    m_canRead = false;
    // return KeywordDetection::create(-1, -1, m_keyword);
    return KeywordDetection::create(14714, 22499, m_keyword);
}

void StressTesterController::onDialogUXStateChanged(DialogUXState newState) {
    Log::info(TAG, "New Dialog State: %s", stateToString(newState).c_str());
    if((newState == DialogUXState::FINISHED || newState == DialogUXState::IDLE) && m_readEnabled) {
        std::async(std::launch::async, std::bind(&StressTesterController::triggerRead, this));
    }
}

void StressTesterController::triggerRead() {
    std::this_thread::sleep_for(m_interval);
    m_canRead = true;
    m_cv.notify_all();
}

void StressTesterController::enableReading() {
    m_readEnabled = true;
    std::async(std::launch::async, std::bind(&StressTesterController::triggerRead, this));
}

} // stressTesterApp
} // alexaClientSDK
