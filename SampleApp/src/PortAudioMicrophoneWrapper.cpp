/*
 * PortAudioMicrophoneWrapper.cpp
 *
 * Copyright (c) 2017-2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     http://aws.amazon.com/apache2.0/
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

#include "SampleApp/PortAudioMicrophoneWrapper.h"
#include "SampleApp/ConsolePrinter.h"

namespace alexaClientSDK {
namespace sampleApp {

using avsCommon::avs::AudioInputStream;

static const int NUM_INPUT_CHANNELS = 1;
static const int NUM_OUTPUT_CHANNELS = 0;
static const double SAMPLE_RATE = 16000;
static const unsigned long PREFERRED_SAMPLES_PER_CALLBACK = paFramesPerBufferUnspecified;

/**
 * Simple helper method for printing a message using the @c ConsolePrinter with
 * printing a PortAudio error.
 *
 * @param err PortAudio error
 * @param msg Message to print before printing the error PortAudio message
 */
void printPaError(PaError err, std::string msg) {
    std::ostringstream os;
    os << msg << " : [" << err << "] " << Pa_GetErrorText(err);
    ConsolePrinter::simplePrint(os.str());
}

std::unique_ptr<PortAudioMicrophoneWrapper> PortAudioMicrophoneWrapper::create(
    std::shared_ptr<avsCommon::avs::AudioInputStream::Buffer> buffer,
    std::shared_ptr<AudioInputStream> stream) {
    if (!stream) {
        ConsolePrinter::simplePrint("Invalid stream passed to PortAudioMicrophoneWrapper");
        return nullptr;
    }
    std::unique_ptr<PortAudioMicrophoneWrapper> portAudioMicrophoneWrapper(new PortAudioMicrophoneWrapper(buffer, stream));
    if (!portAudioMicrophoneWrapper->initialize()) {
        ConsolePrinter::simplePrint("Failed to initialize PortAudioMicrophoneWrapper");
        return nullptr;
    }
    return portAudioMicrophoneWrapper;
}

PortAudioMicrophoneWrapper::PortAudioMicrophoneWrapper(std::shared_ptr<avsCommon::avs::AudioInputStream::Buffer> buffer, std::shared_ptr<AudioInputStream> stream) :
        m_audioInputStream{stream},
        m_paStream{nullptr},
        m_buffer(buffer) {
}

PortAudioMicrophoneWrapper::~PortAudioMicrophoneWrapper() {
    Pa_StopStream(m_paStream);
    Pa_CloseStream(m_paStream);
    Pa_Terminate();
}

bool PortAudioMicrophoneWrapper::initialize() {
    m_streaming = false;
    m_writer = m_audioInputStream->createWriter(AudioInputStream::Writer::Policy::NONBLOCKABLE);
    if (!m_writer) {
        ConsolePrinter::simplePrint("Failed to create stream writer");
        return false;
    }
    
#ifdef KWD_HARDWARE
    return true;
#else
    return openStream();
#endif
}

bool PortAudioMicrophoneWrapper::closeStream() {
    PaError err = Pa_CloseStream(m_paStream);
    if(err != paNoError) {
        printPaError(err, "Failed to close PortAudio default stream");
        return false;
    }
    err = Pa_Terminate();
    if(err != paNoError) {
        printPaError(err, "Failed to terminate PortAudio");
        return false;
    }
    return true;
}

bool PortAudioMicrophoneWrapper::openStream() {
    PaError err;
    err = Pa_Initialize();
    if (err != paNoError) {
        printPaError(err, "Failed to initialize PortAudio");
        return false;
    }

    err = Pa_OpenDefaultStream(
        &m_paStream,
        NUM_INPUT_CHANNELS,
        NUM_OUTPUT_CHANNELS,
        paInt16,
        SAMPLE_RATE,
        PREFERRED_SAMPLES_PER_CALLBACK,
        PortAudioCallback,
        this);

    if (err != paNoError) {
        printPaError(err, "Failed to open PortAudio default stream");
        return false;
    }
    return true;
}

bool PortAudioMicrophoneWrapper::startStreamingMicrophoneData() {
    std::lock_guard<std::mutex> lock{m_mutex};
#ifdef KWD_HARDWARE
    if(!openStream()) {
        return false;
    }
#endif
    PaError err = Pa_StartStream(m_paStream);
    if (err != paNoError) {
        printPaError(err, "Failed to start PortAudio stream");
        return false;
    }
    m_streaming = true;
    return true;
}

bool PortAudioMicrophoneWrapper::stopStreamingMicrophoneData() {
    std::lock_guard<std::mutex> lock{m_mutex};
    if(!m_streaming)
        return true;
    PaError err = Pa_StopStream(m_paStream);
    if (err != paNoError) {
        printPaError(err, "Failed to stop PortAudio stream");
        return false;
    }
    m_streaming = false;
#ifdef KWD_HARDWARE
    return closeStream();
#else
    return true;
#endif
}

int PortAudioMicrophoneWrapper::PortAudioCallback(
    const void* inputBuffer,
    void* outputBuffer,
    unsigned long numSamples,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void* userData) {
    PortAudioMicrophoneWrapper* wrapper = static_cast<PortAudioMicrophoneWrapper*>(userData);
    ssize_t returnCode = wrapper->m_writer->write(inputBuffer, numSamples);
    if (returnCode <= 0) {
        ConsolePrinter::simplePrint("Failed to write to stream.");
        return paAbort;
    }
    return paContinue;
}

bool PortAudioMicrophoneWrapper::isStreaming() {
    return m_streaming;
}

}  // namespace sampleApp
}  // namespace alexaClientSDK
