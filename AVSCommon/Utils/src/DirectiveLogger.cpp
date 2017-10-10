/**
 * DirectiveLogger.cpp
 *
 * TODO: Add Intel copyright
 */

#include "AVSCommon/Utils/TestLogger/TestLogger.h"
#include "AVSCommon/Utils/TestLogger/DirectiveLogger.h"

#define TAG "DirectiveLogger"

namespace alexaClientSDK {
namespace avsCommon {
namespace utils {
namespace testLogger {

    using namespace avs;

/// The namespace for this capability agent.
static const std::string NAMESPACE = "DirectiveLogger";

/// The StopCapture directive signature.
static const avsCommon::avs::NamespaceAndName STOP_CAPTURE{NAMESPACE, "StopCapture"};

/// The ExpectSpeech directive signature.
static const avsCommon::avs::NamespaceAndName EXPECT_SPEECH{NAMESPACE, "ExpectSpeech"};

/// The SetAlert directive signature.
static const avsCommon::avs::NamespaceAndName SET_ALERT{NAMESPACE, "SetAlert"};
/// The DeleteAlert directive signature.
static const avsCommon::avs::NamespaceAndName DELETE_ALERT{NAMESPACE, "DeleteAlert"};

/// The @c Play directive signature.
static const NamespaceAndName PLAY{NAMESPACE, "Play"};

/// The @c Stop directive signature.
static const NamespaceAndName STOP{NAMESPACE, "Stop"};

/// The @c Speak directive signature.
static const NamespaceAndName SPEAK{NAMESPACE, "Speak"};

std::shared_ptr<DirectiveLogger> DirectiveLogger::create(
        std::shared_ptr<sdkInterfaces::ExceptionEncounteredSenderInterface> exc) {
    return std::shared_ptr<DirectiveLogger>(new DirectiveLogger(exc));
}

void DirectiveLogger::handleDirectiveImmediately(
        std::shared_ptr<avsCommon::avs::AVSDirective> directive)
{
    handleDirective(std::make_shared<DirectiveInfo>(directive, nullptr));
}

void DirectiveLogger::preHandleDirective(std::shared_ptr<DirectiveInfo> info) {
    Log::info(TAG, "preHandleDirective: %s", info->directive->getUnparsedDirective().c_str());
}

void DirectiveLogger::handleDirective(std::shared_ptr<DirectiveInfo> info) {
    Log::info(TAG, "handleDirective: %s", info->directive->getUnparsedDirective().c_str());
}

void DirectiveLogger::cancelDirective(std::shared_ptr<DirectiveInfo> info) {
    Log::info(TAG, "cancelDirective: %s", info->directive->getUnparsedDirective().c_str());
}
void DirectiveLogger::onDeregistered() {
    Log::info(TAG, "onDeregistered");
}

avsCommon::avs::DirectiveHandlerConfiguration DirectiveLogger::getConfiguration() const {
    avsCommon::avs::DirectiveHandlerConfiguration configuration;
    configuration[STOP_CAPTURE] = avsCommon::avs::BlockingPolicy::NON_BLOCKING;
    configuration[EXPECT_SPEECH] = avsCommon::avs::BlockingPolicy::NON_BLOCKING;
    configuration[SET_ALERT] = avsCommon::avs::BlockingPolicy::NON_BLOCKING;
    configuration[DELETE_ALERT] = avsCommon::avs::BlockingPolicy::NON_BLOCKING;
    configuration[PLAY] = avsCommon::avs::BlockingPolicy::NON_BLOCKING;
    configuration[STOP] = avsCommon::avs::BlockingPolicy::NON_BLOCKING;
    configuration[SPEAK] = avsCommon::avs::BlockingPolicy::NON_BLOCKING;
    return configuration;
}

DirectiveLogger::DirectiveLogger(
        std::shared_ptr<sdkInterfaces::ExceptionEncounteredSenderInterface> exc) :
    CapabilityAgent{NAMESPACE, exc}
{}

// DirectiveLogger::~DirectiveLogger() {}

} // testLogger
} // utils
} // avsCommon
} // alexaClientSDK
