/**
 * AudioFilePlayer.cpp
 *
 * TODO: Add Intel copyright
 */

#include <fstream>
#include <iterator>

#include "StressTesterApp/Logger.h"
#include "StressTesterApp/AudioFilePlayer.h"

#define TAG "AudioFilePlayer"
#define MAX_INPUT_SIZE 320

namespace alexaClientSDK {
namespace stressTesterApp {

std::shared_ptr<AudioFilePlayer> AudioFilePlayer::create(std::string audioFile) {
    // std::vector<uint8_t> bytes;
    // char ch;
    // int len = 0;
    // FILE* fp;

    // if((fp = fopen(audioFile.c_str(), "r")) == NULL) {
    //     Log::error(TAG, "Failed to open \"%s\"", audioFile.c_str());
    //     return nullptr;
    // }

    // while((ch = fgetc(fp)) != EOF) {
    //     bytes.push_back((uint8_t) ch);
    //     len++;
    // }
    std::ifstream input(audioFile, std::ios::binary);
    if(!input.good()) {
        Log::error(TAG, "Failed to open \"%s\"", audioFile.c_str());
        return nullptr;
    }

    std::vector<char> bytes((std::istreambuf_iterator<char>(input)),
                                std::istreambuf_iterator<char>());

    int len = bytes.size();
    uint8_t* data = new uint8_t[len];

    for(int i = 0; i < len; i++) {
        data[i] = bytes[i];
    }

    // delete[] data;
    Log::debug(TAG, "Number of bytes: %d", bytes.size());

    return std::shared_ptr<AudioFilePlayer>(new AudioFilePlayer(data, len));
}


void AudioFilePlayer::onKeyWordDetected(
        std::shared_ptr<AudioInputStream> stream,
        std::string keyword,
        AudioInputStream::Index begin,
        AudioInputStream::Index end)
{
    Log::info(TAG, "Key word was detected, writing data to the SDS");
    if(!m_streamWriter) {
        m_streamWriter = stream->createWriter(AudioInputStream::Writer::Policy::NONBLOCKABLE);
    }
    // Log::debug(TAG, "Number of bytes: %d", m_audioData.size());
    // char data[m_audioData.size()];
    // Log::debug(TAG, "Looping....");
    // for(unsigned int i = 0; i < m_audioData.size(); i++) {
    //     Log::debug(TAG, "i = %d : %d", i, (uint8_t) m_audioData[i]);
    //     data[i] = m_audioData[i];
    // }
    // Log::info(TAG, "Done");
    for(int i = 0; i < m_audioDataLen; i++) {
        Log::debug(TAG, "Byte %d: %d", i, m_audioData[i]);
    }

    if(m_streamWriter->write(m_audioData, m_audioDataLen) <= 0) {
    // if(m_streamWriter->write(&data, m_audioData.size()) <= 0) {
        Log::error(TAG, "Failed to write to the stream");
    }
}

AudioFilePlayer::AudioFilePlayer(uint8_t* audioData, int audioDataLen) :
    m_audioData(audioData), m_audioDataLen(audioDataLen), 
    m_streamWriter(nullptr)
{}

AudioFilePlayer::~AudioFilePlayer() {
    if(m_streamWriter) {
        m_streamWriter->close();
    }
    delete[] m_audioData;
}

} // stressTesterApp
} // alexaClientSDK
