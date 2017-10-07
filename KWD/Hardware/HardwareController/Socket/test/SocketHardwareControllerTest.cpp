
#include <stdlib.h>
#include <stdio.h>

#include <chrono>

#include "Socket/SocketHardwareController.h"

using namespace alexaClientSDK::kwd;

int main(int argc, char** argv) {
    std::chrono::milliseconds timeout(5000);
    std::unique_ptr<SocketHardwareController> ctrl = SocketHardwareController::create(
            "127.0.0.1", 3000);
    if(ctrl == nullptr) {
        fprintf(stderr, "Failed to connect to host\n");
        return 0;
    }

    while(true) {
        auto ret = ctrl->read(timeout);
        if(!ret) continue;
        fprintf(stderr, "Received keyword: %s\n", ret->getKeyword().c_str());
    }
    
    return 0;
}
