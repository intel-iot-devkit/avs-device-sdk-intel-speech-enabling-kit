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

/// Conditional variable used for syncronization between triggeredRead() and read()
static std::condition_variable g_cv;

/// Boolean flag to signal that it is time to trigger the keyword.
static std::atomic<bool> g_canRead;


std::shared_ptr<StressTesterController> StressTesterController::create(
        std::chrono::milliseconds interval, std::string keyword) {
    return std::make_shared<StressTesterController>(StressTesterController(
                interval, keyword));
}

StressTesterController::StressTesterController(
        std::chrono::milliseconds interval, std::string keyword) :
    m_interval(interval), m_keyword(keyword)
{}

StressTesterController::~StressTesterController() {}

std::unique_ptr<KeywordDetection> StressTesterController::read(
        std::chrono::milliseconds timeout) 
{
    if(!g_canRead) {
        std::mutex mutCv;
        std::unique_lock<std::mutex> lk(mutCv);
        auto now = std::chrono::system_clock::now();

        if(g_cv.wait_until(lk, now + timeout) == std::cv_status::timeout) {
            return nullptr;
        }
    }

    g_canRead = false;
    return KeywordDetection::create(-1, -1, m_keyword);
}

void StressTesterController::onDialogUXStateChanged(DialogUXState newState) {
    Log::info("New Dialog State: %s", stateToString(newState).c_str());
    if(newState == DialogUXState::FINISHED || newState == DialogUXState::IDLE) {
        std::async(std::launch::async, std::bind(&StressTesterController::triggerRead, this));
    }
}

void StressTesterController::triggerRead() {
    std::this_thread::sleep_for(m_interval);
    g_canRead = true;
    g_cv.notify_all();
}

} // stressTesterApp
} // alexaClientSDK
