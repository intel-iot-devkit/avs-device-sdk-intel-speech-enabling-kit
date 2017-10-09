/**
 * Arguments.cpp
 *
 * TODO: Add Intel copyright
 */

#include <stdlib.h>

#include "StressTesterApp/Logger.h"
#include "StressTesterApp/Arguments.h"

#define TAG "Arguments"

namespace alexaClientSDK {
namespace stressTesterApp {

std::shared_ptr<Arguments> Arguments::parse(int argc, char** argv) {
    if(argc < 4) {
        Log::error(TAG, "Too few arguments");
        usage(argv[0]);
        return nullptr;
    } else if(argc > 4) {
        Log::error(TAG, "Too many arguments");
        usage(argv[0]);
        return nullptr;
    }

    std::string audioFile = std::string(argv[1]);
    std::string keyword = std::string(argv[2]);
    std::chrono::milliseconds interval = std::chrono::milliseconds(atoi(argv[3]));
    
    return std::make_shared<Arguments>(Arguments(audioFile, keyword, interval));
}

void Arguments::usage(const char* name) {
    fprintf(stderr, "usage: %s <audio-file> <keyword> <interval>\n", name);
    fprintf(stderr, "\taudio-file - Utterance audio file to pass to alexa\n");
    fprintf(stderr, "\tkeyword    - Keyword which will intiate the query\n");
    fprintf(stderr, "\tinterval   - Interval in ms after finished directive to send again\n");
}

Arguments::Arguments(std::string audioFile, std::string keyword, std::chrono::milliseconds interval) :
    m_audioFile(audioFile), m_keyword(keyword), m_interval(interval)
{}

std::string Arguments::getAudioFile() {
    return m_audioFile;
}

std::string Arguments::getKeyword() {
    return m_keyword;
}

std::chrono::milliseconds Arguments::getInterval() {
    return m_interval;
}
    
} // stressTesterApp
} // alexaClientSDK
