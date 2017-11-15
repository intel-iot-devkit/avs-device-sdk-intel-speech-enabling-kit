/**
 * AbstractHardwareController.h
 *
 * TODO: Add Intel copyright
 */

#ifndef ALEXA_CLIENT_SDK_KWD_HW_ABSTRACT_CTRL_H_
#define ALEXA_CLIENT_SDK_KWD_HW_ABSTRACT_CTRL_H_

#include <chrono>
#include <memory>

#include "Hardware/KeywordDetection.h"

namespace alexaClientSDK {
namespace kwd {

class AbstractHardwareController {
public:
    /**
     * Read a @c KeywordDetection from the hardware controller.
     *
     * @param timeout Timeout for the read
     * @return @c KeywordDetection when a detection occurs, otherwise @c nullptr
     * if an error occurs, or a timeout
     */
    virtual std::unique_ptr<KeywordDetection> read(std::chrono::milliseconds timeout) = 0;

    /**
     * Destructor.
     */
    virtual ~AbstractHardwareController() = default;
};

} // kwd
} // alexaClientSDK

#endif // ALEXA_CLIENT_SDK_KWD_HW_ABSTRACT_CTRL_H_
