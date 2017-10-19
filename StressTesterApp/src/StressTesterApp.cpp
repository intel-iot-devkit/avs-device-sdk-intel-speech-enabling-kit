/**
 * StressTesterApp.cpp
 *
 * TODO: Add Intel copyright
 */
#include <fstream>
#include <thread>
#include <chrono>

#include <AVSCommon/AVS/Initialization/AlexaClientSDKInit.h>
#include <ACL/Transport/HTTPContentFetcherFactory.h>
#include <Alerts/Storage/SQLiteAlertStorage.h>
#include <Settings/SQLiteSettingStorage.h>
#include <AuthDelegate/AuthDelegate.h>
#include <AVSCommon/AVS/Initialization/AlexaClientSDKInit.h>
#include <AVSCommon/Utils/Logger/LoggerSinkManager.h>
#include <MediaPlayer/MediaPlayer.h>
#include <DefaultClient/DefaultClient.h>
#include <Hardware/HardwareKeywordDetector.h>

#include "StressTesterApp/StressTesterApp.h"
#include "StressTesterApp/StressTesterController.h"
#include "StressTesterApp/Arguments.h"
#include "StressTesterApp/Logger.h"
#include "StressTesterApp/ConnectionObserver.h"
#include "StressTesterApp/KeywordObserver.h"
#include "StressTesterApp/AudioFilePlayer.h"

#define TAG "StressTesterApp"

namespace alexaClientSDK {
namespace stressTesterApp {

using namespace avsCommon;
using namespace avsCommon::avs;
using namespace alexaClientSDK::mediaPlayer;
using namespace alexaClientSDK::capabilityAgents;

/// The sample rate of microphone audio data.
static const unsigned int SAMPLE_RATE_HZ = 16000;

/// The number of audio channels.
static const unsigned int NUM_CHANNELS = 1;

/// The size of each word within the stream.
static const size_t WORD_SIZE = 2;

/// The maximum number of readers of the stream.
static const size_t MAX_READERS = 10;

/// The amount of audio data to keep in the ring buffer.
static const std::chrono::seconds AMOUNT_OF_AUDIO_DATA_IN_BUFFER = std::chrono::seconds(15);

/// The size of the ring buffer.
static const size_t BUFFER_SIZE_IN_SAMPLES = (SAMPLE_RATE_HZ)*AMOUNT_OF_AUDIO_DATA_IN_BUFFER.count();

std::unique_ptr<StressTesterApp> StressTesterApp::create(
        std::shared_ptr<Arguments> args)
{
    auto app = std::unique_ptr<StressTesterApp>(new StressTesterApp);
    if(!app->initialize(args)) {
        Log::error(TAG, "Failed to initialize the StressTesterApp");
        return nullptr;
    } else {
        Log::info(TAG, "StressTesterApp initialized");
    }
    return app;
}

StressTesterApp::StressTesterApp() :
    m_keywordDetector(nullptr)
{}

void StressTesterApp::run() {
    while(true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

bool StressTesterApp::initialize(std::shared_ptr<Arguments> args) {
    // TODO: Initialize SDK logging
    

    std::ifstream configInFile(args->getConfigFile());
    if(!configInFile.good()) {
        Log::error(TAG, "Failed to read config file: %s", args->getConfigFile().c_str());
        return false;
    } 

    if(!initialization::AlexaClientSDKInit::initialize({&configInFile})) {
        Log::error(TAG, "Failed to initialize SDK!");
        return false;
    }

    auto httpContentFetcherFactory = std::make_shared<acl::HTTPContentFetcherFactory>();

     // Creating the media players. Here, the default GStreamer based 
     // MediaPlayer is being created. However, any MediaPlayer that follows 
     // the specified MediaPlayerInterface can work.
    auto speakMediaPlayer = MediaPlayer::create( httpContentFetcherFactory);
    if (!speakMediaPlayer) {
        Log::error(TAG, "Failed to create media player for speech!");
        return false;
    }

    auto audioMediaPlayer = MediaPlayer::create(httpContentFetcherFactory);
    if (!audioMediaPlayer) {
        Log::error(TAG, "Failed to create media player for content!");
        return false;
    }

    auto alertsMediaPlayer = MediaPlayer::create(httpContentFetcherFactory);
    if (!alertsMediaPlayer) {
        Log::error(TAG, "Failed to create media player for alerts!");
        return false;
    }

    // Creating the alert storage object to be used for rendering and storing alerts.
    auto alertStorage = std::make_shared<alerts::storage::SQLiteAlertStorage>();

     // Creating settings storage object to be used for storing <key, value> 
     // pairs of AVS Settings.
    auto settingsStorage = std::make_shared<settings::SQLiteSettingStorage>();

     // Setting up a connection observer to wait for connection and 
     // authorization prior to accepting user input at startup.
    auto connectionObserver = std::make_shared<ConnectionObserver>();

     // Creating the AuthDelegate - this component takes care of LWA and 
     // authorization of the client. At the moment, this must be done and 
     // authorization must be achieved prior to making the call to connect().
    std::shared_ptr<alexaClientSDK::authDelegate::AuthDelegate> authDelegate =
        alexaClientSDK::authDelegate::AuthDelegate::create();

    authDelegate->addAuthObserver(connectionObserver);

    // Initializing the stress tester controller
    auto controller = StressTesterController::create(
            args->getInterval(), args->getKeyword());

    if(!controller) {
        Log::error(TAG, "Failed to initialize the stress tester controller");
        return false;
    }

    // Creating the DefaultClient - this component serves as an out-of-box
    // default object that instantiates and "glues" together all the modules.
    std::shared_ptr<alexaClientSDK::defaultClient::DefaultClient> client =
        alexaClientSDK::defaultClient::DefaultClient::create(
            speakMediaPlayer,
            audioMediaPlayer,
            alertsMediaPlayer,
            authDelegate,
            alertStorage,
            settingsStorage,
            {controller},
            {connectionObserver});

    if (!client) {
        Log::error(TAG, "Failed to create default SDK client!");
        return false;
    }

     // TODO: ACSDK-384 Remove the requirement of clients having to wait for 
     // authorization before making the connect() call.
    if (!connectionObserver->waitFor(
            sdkInterfaces::AuthObserverInterface::State::REFRESHED)) {
        Log::error(TAG, "Failed to authorize SDK client!");
        return false;
    }

    client->connect();

    if (!connectionObserver->waitFor(
                sdkInterfaces::ConnectionStatusObserverInterface::Status::CONNECTED)) {
        Log::error(TAG, "Failed to connect to AVS!");
        return false;
    }

    // Send default settings set by the user to AVS.
    client->sendDefaultSettings();

    // Creating the buffer (Shared Data Stream) that will hold user audio data.
    // This is the main input into the SDK.
    size_t bufferSize = AudioInputStream::calculateBufferSize(
        BUFFER_SIZE_IN_SAMPLES, WORD_SIZE, MAX_READERS);
    auto buffer = std::make_shared<AudioInputStream::Buffer>(bufferSize);
    std::shared_ptr<AudioInputStream> sharedDataStream = AudioInputStream::create(
            buffer, WORD_SIZE, MAX_READERS);

    if (!sharedDataStream) {
        Log::error(TAG, "Failed to create shared data stream!");
        return false;
    }

    alexaClientSDK::avsCommon::utils::AudioFormat compatibleAudioFormat;
    compatibleAudioFormat.sampleRateHz = SAMPLE_RATE_HZ;
    compatibleAudioFormat.sampleSizeInBits = WORD_SIZE * CHAR_BIT;
    compatibleAudioFormat.numChannels = NUM_CHANNELS;
    compatibleAudioFormat.endianness = utils::AudioFormat::Endianness::LITTLE;
    compatibleAudioFormat.encoding = utils::AudioFormat::Encoding::LPCM;

    capabilityAgents::aip::AudioProvider wwAudioProvider(
        sharedDataStream,
        compatibleAudioFormat,
        alexaClientSDK::capabilityAgents::aip::ASRProfile::NEAR_FIELD,
        true, true, true);
    
    // Initializing the hardware keyword observer
    auto audioFilePlayer = AudioFilePlayer::create(args->getAudioFile());

    if(!audioFilePlayer) {
        Log::error(TAG, "Failed to initialize the audio file player!");
        return false;
    }

    // This observer is notified any time a keyword is detected and notifies 
    // the DefaultClient to start recognizing.
    auto keywordObserver = std::make_shared<KeywordObserver>(client, wwAudioProvider);
    
    // Intialize keyword detector
    m_keywordDetector = kwd::HardwareKeywordDetector::create(
            sharedDataStream, compatibleAudioFormat, controller,
            {audioFilePlayer, keywordObserver},
            std::unordered_set<std::shared_ptr<
                sdkInterfaces::KeyWordDetectorStateObserverInterface>>());

    if(!m_keywordDetector) {
        Log::error(TAG, "Failed to initialize the keyword detector");
        return false;
    }

    controller->enableReading();

    return true;
}

} // stressTesterApp
} // alexaClientSDK
