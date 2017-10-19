
#include <AVSCommon/SDKInterfaces/DialogUXStateObserverInterface.h>
#include "StressTesterApp/Arguments.h"
#include "StressTesterApp/StressTesterController.h"
#include "StressTesterApp/StressTesterApp.h"
#include "StressTesterApp/Logger.h"

#define TAG "Main"

using namespace alexaClientSDK::stressTesterApp;
using namespace alexaClientSDK::avsCommon::sdkInterfaces;

std::shared_ptr<StressTesterApp> g_app;

// void signal_handler(int signum) {
//     g_app->stop();
// }

int main(int argc, char** argv) {
    auto args = Arguments::parse(argc, argv);
    if(!args) { return -1; }

    auto stapp = StressTesterApp::create(args);

    if(!stapp) {
        Log::error(TAG, "Failed to initialize the StressTesterApp!");
        return -1;
    }

    Log::info(TAG, "Application initialized");
    stapp->run();

    return 0;
}
