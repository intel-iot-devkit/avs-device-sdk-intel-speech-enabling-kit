
#include <AVSCommon/SDKInterfaces/DialogUXStateObserverInterface.h>
#include "StressTesterApp/Arguments.h"
#include "StressTesterApp/StressTesterController.h"

using namespace alexaClientSDK::stressTesterApp;
using namespace alexaClientSDK::avsCommon::sdkInterfaces;

int main(int argc, char** argv) {
    auto args = Arguments::parse(argc, argv);
    auto stc = StressTesterController::create(args->getInterval(), args->getKeyword());
    stc->onDialogUXStateChanged(DialogUXStateObserverInterface::DialogUXState::IDLE);
    auto kw = stc->read(std::chrono::milliseconds(2000));
    if(kw)
        printf("Got kw: %s\n", kw->getKeyword().c_str());
    else
        printf("No keyword\n");
    return 0;
}
