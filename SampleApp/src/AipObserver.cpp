/**
 * AipObserver.cpp
 *
 * TODO: Add Intel copyright
 */

#include <sstream>

#include "SampleApp/ConsolePrinter.h"
#include "SampleApp/AipObserver.h"

namespace alexaClientSDK {
namespace sampleApp {

std::shared_ptr<AipObserver> AipObserver::create() {
    return std::make_shared<AipObserver>(AipObserver());
}

void AipObserver::onStateChanged(State state) {
    std::ostringstream os;
    os << "-- AudioInputProcessor State Changed: " 
        << AudioInputProcessorObserverInterface::stateToString(state);
    ConsolePrinter::simplePrint(os.str());
}

AipObserver::AipObserver() {}

} // sampleApp
} // alexaClientSDK
