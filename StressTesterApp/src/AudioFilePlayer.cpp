/**
 * AudioFilePlayer.cpp
 *
 * TODO: Add Intel copyright
 */

#include <stdlib.h>
#include <stdio.h>

#include "StressTesterApp/Logger.h"
#include "StressTesterApp/AudioFilePlayer.h"

#define TAG "AudioFilePlayer"
#define MAX_INPUT_SIZE 320

namespace alexaClientSDK {
namespace stressTesterApp {

std::shared_ptr<AudioFilePlayer> AudioFilePlayer::create(std::string audioFile) {
    std::vector<uint8_t> bytes;
    char ch;
    int len = 0;
    FILE* fp;

    if((fp = fopen(audioFile.c_str(), "r")) == NULL) {
        Log::error(TAG, "Failed to open \"%s\"", audioFile.c_str());
        return nullptr;
    }

    while((ch = fgetc(fp)) != EOF && len < MAX_INPUT_SIZE) {
        bytes.push_back((uint8_t) ch);
        len++;
    }

    return std::make_shared<AudioFilePlayer>(AudioFilePlayer(bytes));
}


void AudioFilePlayer::onKeyWordDetected(
        std::shared_ptr<AudioInputStream> stream,
        std::string keyword,
        AudioInputStream::Index begin,
        AudioInputStream::Index end)
{
    Log::info(TAG, "Key word was detected, writing data to the SDS");
    auto writer = stream->createWriter(AudioInputStream::Writer::Policy::NONBLOCKABLE);
    if(writer->write(&m_audioData[0], m_audioData.size()) <= 0) {
        Log::error(TAG, "Failed to write to the stream");
    }
}

AudioFilePlayer::AudioFilePlayer(std::vector<uint8_t> audioData) :
    m_audioData(audioData)
{}

AudioFilePlayer::~AudioFilePlayer() {}

} // stressTesterApp
} // alexaClientSDK
