
#include <stdlib.h>
#include <stdio.h>

#include "Socket/SocketHardwareController.h"

using namespace alexaClientSDK::kwd;

int main(int argc, char** argv) {
    std::unique_ptr<SocketHardwareController> ctrl = SocketHardwareController::create("localhost", 3000);
    if(ctrl == nullptr) {
        fprintf(stderr, "Failed to connect to host\n");
        return 0;
    }

    return 0;
}
