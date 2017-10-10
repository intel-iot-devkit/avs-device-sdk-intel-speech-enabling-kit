/**
 * DirectiveLogger.h
 *
 * TODO: Add Intel copyright
 */

#ifndef ALEXA_CLIENT_SDK_DIRECTIVE_LOGGER_H_
#define ALEXA_CLIENT_SDK_DIRECTIVE_LOGGER_H_

#include <memory>

#include "AVSCommon/AVS/CapabilityAgent.h"
#include "AVSCommon/AVS/DirectiveHandlerConfiguration.h"
#include "AVSCommon/SDKInterfaces/DirectiveHandlerInterface.h"

namespace alexaClientSDK {
namespace avsCommon {
namespace utils {
namespace testLogger {

using namespace avsCommon::sdkInterfaces;
using namespace avsCommon::avs;

class DirectiveLogger : public CapabilityAgent {
public:

    static std::shared_ptr<DirectiveLogger> create(
        std::shared_ptr<sdkInterfaces::ExceptionEncounteredSenderInterface> exc);

    void handleDirectiveImmediately(
            std::shared_ptr<avsCommon::avs::AVSDirective> directive) override;
    void preHandleDirective(std::shared_ptr<DirectiveInfo> info) override;
    void handleDirective(std::shared_ptr<DirectiveInfo> info) override;
    void cancelDirective(std::shared_ptr<DirectiveInfo> info) override;
    void onDeregistered() override;

    DirectiveHandlerConfiguration getConfiguration() const override;

private:
    DirectiveLogger(
        std::shared_ptr<sdkInterfaces::ExceptionEncounteredSenderInterface> exc);
};

} // testLogger
} // utils
} // avsCommon
} // alexaClientSDK
#endif // ALEXA_CLIENT_SDK_DIRECTIVE_LOGGER_H_
