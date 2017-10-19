/**
 * StressTesterApp.g
 * 
 * TODO: Add Intel copyright
 */

#ifndef ALEXA_CLIENT_SDK_STRESS_TESTER_INCLUDE_STRESS_TESTER_APP_
#define ALEXA_CLIENT_SDK_STRESS_TESTER_INCLUDE_STRESS_TESTER_APP_

#include <memory>

#include <Hardware/HardwareKeywordDetector.h>

#include "StressTesterApp/Arguments.h"

namespace alexaClientSDK {
namespace stressTesterApp {

class StressTesterApp {
public:
    /**
     * Creates a StressTesterApp
     *
     * @param args - Command line arguments
     * @return SampleStressApp pointer, or nullptr if an error occurred
     */
    static std::unique_ptr<StressTesterApp> create(std::shared_ptr<Arguments> args);

    /// Runs the application, blocking until the user quits
    void run();

    /**
     * Destructor.
     */
    ~StressTesterApp() = default;

private:
    /**
     * Contructor.
     */
    StressTesterApp();

    /**
     * Initialize the SampleTesterApp.
     *
     * @param args - Command line arguments
     * @return @c true if initialization succeeded, @c false otherwise
     */
    bool initialize(std::shared_ptr<Arguments> args); 

    // Keyword detector
    std::shared_ptr<kwd::HardwareKeywordDetector> m_keywordDetector;
};

} // stressTesterApp
} // alexaClientSDK

#endif
