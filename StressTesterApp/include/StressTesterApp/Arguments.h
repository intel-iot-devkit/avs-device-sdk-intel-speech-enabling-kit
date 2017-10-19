/**
 * Arguments.h
 *
 * TODO: Add Intel copyright
 */

#ifndef ALEXA_CLIENT_SDK_STRESS_TESTER_INCLUDE_ARGUMENTS_
#define ALEXA_CLIENT_SDK_STRESS_TESTER_INCLUDE_ARGUMENTS_

#include <memory>
#include <chrono>
#include <string>
#include <mutex>

namespace alexaClientSDK {
namespace stressTesterApp {

class Arguments {
public:
    /**
     * Parse the command line argumenst into an @c Arguments object.
     *
     * @param argc Argument cound
     * @param argv Arguments
     * @return @c Arguments, or nullptr if an error occurs
     */
    static std::shared_ptr<Arguments> parse(int argc, char** argv);

    /**
     * Get the path to the audio file.
     */
    std::string getAudioFile();

    /**
     * Get the keyword.
     */
    std::string getKeyword();

    /**
     * Get the interval.
     */
    std::chrono::milliseconds getInterval();

    /**
     * Get the configuration file for the Alexa SDK.
     */
    std::string getConfigFile();

private:
    /**
     * Print the usage for the application.
     *
     * @param name Name of the application
     */
    static void usage(const char* name);

    /**
     * Constructor.
     *
     * @param audioFile Sound file to load
     * @param keyword Keyword
     * @param interval Interval
     * @param configFile Alexa SDK configuration file path
     */
    Arguments(std::string audioFile, std::string keyword, 
            std::chrono::milliseconds interval,
            std::string configFile);

    /// Sound file to load as the voice input for Alexa
    std::string m_audioFile;

    /// Keyword to initate the query with
    std::string m_keyword;

    /// Interval to run send another command to Alexa after the completion
    /// of a directive.
    std::chrono::milliseconds m_interval;

    /// Alexa SDK config file
    std::string m_configFile;
};

} // stressTesterApp
} // alexaClientSDK

#endif // ALEXA_CLIENT_SDK_STRESS_TESTER_INCLUDE_ARGUMENTS_

