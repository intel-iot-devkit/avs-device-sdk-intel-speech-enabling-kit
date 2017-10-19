/**
 * AudioFilePlayer.h
 *
 * TODO: Add Intel copyright
 */

#ifndef ALEXA_CLIENT_SDK_STRESS_TESTER_INCLUDE_AUDIO_FILE_PLAYER_
#define ALEXA_CLIENT_SDK_STRESS_TESTER_INCLUDE_AUDIO_FILE_PLAYER_

#include <memory>

#include <AVSCommon/SDKInterfaces/KeyWordObserverInterface.h>
#include <AVSCommon/AVS/AudioInputStream.h>

namespace alexaClientSDK {
namespace stressTesterApp {

using namespace avsCommon::avs;
using namespace avsCommon::sdkInterfaces;

class AudioFilePlayer : public KeyWordObserverInterface {
public:
    /**
     * Creates a new @c AudioFilePlayer.
     *
     * @param audioFile Audio file to read in for playback
     * @return @c AudioFilePlayer, @c nullptr if an error occurs
     */
    static std::shared_ptr<AudioFilePlayer> create(std::string audioFile);

    /**
     * Callback for when a keyword is detected.
     */
    void onKeyWordDetected(
            std::shared_ptr<AudioInputStream> stream,
            std::string keyword,
            AudioInputStream::Index begin = KeyWordObserverInterface::UNSPECIFIED_INDEX,
            AudioInputStream::Index end = KeyWordObserverInterface::UNSPECIFIED_INDEX) override;

    /**
     * Destructor.
     */
    ~AudioFilePlayer() override;

private:
    /**
     * Constructor.
     *
     * @param audioData Raw audio data to pass to the @c AudioInputStream
     */
    AudioFilePlayer(uint8_t* audioData, int audioDataLen);

    /// Audio data to pass to the @c AudioInputStream
    // std::vector<char> m_audioData;
    uint8_t* m_audioData;
    int m_audioDataLen;

    std::unique_ptr<AudioInputStream::Writer> m_streamWriter;
};

} // stressTesterApp
} // alexaClientSDK

#endif // ALEXA_CLIENT_SDK_STRESS_TESTER_INCLUDE_AUDIO_FILE_PLAYER_
