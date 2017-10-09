/**
 * Simple logging class which logs to stderr.
 *
 * @author Kevin Midkiff
 */

#ifndef ALEXA_CLIENT_SDK_STRESS_TESTER_INCLUDE_LOGGER_
#define ALEXA_CLIENT_SDK_STRESS_TESTER_INCLUDE_LOGGER_

#include <stdio.h>
#include <stdarg.h>

#define LOG_TYPE_DEBUG "DEBUG"
#define LOG_TYPE_INFO  "INFO"
#define LOG_TYPE_WARN  "WARN"
#define LOG_TYPE_ERROR "ERROR"
namespace alexaClientSDK {
namespace stressTesterApp {

/**
 * Log class. This class should only be used statically. It is
 * also important to note that each method uses the same formatting
 * as printf.
 */
class Log
{
private:
    /**
     * Base log method which handles the actual print.
     */
    static void log(const char* tag, const char* type, const char* format, va_list args)
    {
        fprintf(stderr, "%-5s - %s : ", type, tag);
        vfprintf(stderr, format, args);
        fprintf(stderr, "\n");
    };

public:

    /**
     * Print a debug message.
     *
     * \note{This will only be printed if the DEBUG definition is defined.}
     */
    static void debug(const char* tag, const char* format, ...)
    {
#ifdef DEBUG
        va_list args;
        va_start(args, format);
        log(tag, LOG_TYPE_DEBUG, format, args);
        va_end(args);
#endif
    };

    /**
     * Print an information message.
     */
    static void info(const char* tag, const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        log(tag, LOG_TYPE_INFO, format, args);
        va_end(args);
    };

    /**
     * Print a warning message.
     */
    static void warn(const char* tag, const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        log(tag, LOG_TYPE_WARN, format, args);
        va_end(args);
    };

    /**
     * Print an error message.
     */
    static void error(const char* tag, const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        log(tag, LOG_TYPE_ERROR, format, args);
        va_end(args);
    };
};
} // stressTesterApp
} // alexaClientSDK
#endif  // ALEXA_CLIENT_SDK_STRESS_TESTER_INCLUDE_LOGGER_
