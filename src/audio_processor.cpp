#include "audio_processor.h"
#include <cmath>
#include <algorithm>
#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

AudioProcessor::AudioProcessor(int sampleRate, int bufferSize)
    : sampleRate(sampleRate), bufferSize(bufferSize), stream(nullptr),
      microphoneActive(false), microphoneInitialized(false) {
    numBins = bufferSize / 2;
    micBuffer.resize(bufferSize, 0.0f);
}

AudioProcessor::~AudioProcessor() {
    stopMicrophone();
    if (microphoneInitialized) {
        Pa_Terminate();
    }
}

void AudioProcessor::generateSyntheticAudio(std::vector<float>& buffer, float time) {
    buffer.resize(bufferSize);
    
    // Generate multiple sine waves with different frequencies
    for (int i = 0; i < bufferSize; i++) {
        float t = time + (float)i / sampleRate;
        float sample = 0.0f;
        
        // Add multiple frequency components
        sample += 0.3f * std::sin(2.0f * M_PI * 220.0f * t);  // A3
        sample += 0.2f * std::sin(2.0f * M_PI * 440.0f * t);  // A4
        sample += 0.15f * std::sin(2.0f * M_PI * 880.0f * t); // A5
        sample += 0.1f * std::sin(2.0f * M_PI * 1760.0f * t); // A6
        
        // Add some bass
        sample += 0.25f * std::sin(2.0f * M_PI * 110.0f * t * (1.0f + 0.05f * std::sin(2.0f * M_PI * 0.5f * t)));
        
        buffer[i] = sample;
    }
}

void AudioProcessor::performFFT(const std::vector<float>& input, std::vector<float>& magnitudes) {
    // Prepare data for FFT
    std::vector<float> real = input;
    std::vector<float> imag(bufferSize, 0.0f);
    
    // Ensure buffer is power of 2
    if ((bufferSize & (bufferSize - 1)) != 0) {
        return; // Buffer size must be power of 2
    }
    
    // Perform FFT
    fft(real, imag);
    
    // Calculate magnitudes
    magnitudes.resize(numBins);
    for (int i = 0; i < numBins; i++) {
        magnitudes[i] = std::sqrt(real[i] * real[i] + imag[i] * imag[i]) / bufferSize;
    }
}

void AudioProcessor::fft(std::vector<float>& real, std::vector<float>& imag) {
    int n = real.size();
    
    // Bit reversal
    for (int i = 1, j = 0; i < n; i++) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) {
            j ^= bit;
        }
        j ^= bit;
        
        if (i < j) {
            std::swap(real[i], real[j]);
            std::swap(imag[i], imag[j]);
        }
    }
    
    // Cooley-Tukey FFT
    for (int len = 2; len <= n; len <<= 1) {
        float angle = -2.0f * M_PI / len;
        float wlen_real = std::cos(angle);
        float wlen_imag = std::sin(angle);
        
        for (int i = 0; i < n; i += len) {
            float w_real = 1.0f;
            float w_imag = 0.0f;
            
            for (int j = 0; j < len / 2; j++) {
                float u_real = real[i + j];
                float u_imag = imag[i + j];
                float v_real = real[i + j + len / 2] * w_real - imag[i + j + len / 2] * w_imag;
                float v_imag = real[i + j + len / 2] * w_imag + imag[i + j + len / 2] * w_real;
                
                real[i + j] = u_real + v_real;
                imag[i + j] = u_imag + v_imag;
                real[i + j + len / 2] = u_real - v_real;
                imag[i + j + len / 2] = u_imag - v_imag;
                
                float w_real_temp = w_real * wlen_real - w_imag * wlen_imag;
                w_imag = w_real * wlen_imag + w_imag * wlen_real;
                w_real = w_real_temp;
            }
        }
    }
}

bool AudioProcessor::initializeMicrophone() {
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        std::cerr << "PortAudio initialization failed: " << Pa_GetErrorText(err) << std::endl;
        return false;
    }
    
    microphoneInitialized = true;
    
    PaStreamParameters inputParameters;
    inputParameters.device = Pa_GetDefaultInputDevice();
    
    if (inputParameters.device == paNoDevice) {
        std::cerr << "No default input device found" << std::endl;
        return false;
    }
    
    inputParameters.channelCount = 1; // Mono
    inputParameters.sampleFormat = paFloat32;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo = nullptr;
    
    err = Pa_OpenStream(
        &stream,
        &inputParameters,
        nullptr, // No output
        sampleRate,
        bufferSize,
        paClipOff,
        audioCallback,
        this
    );
    
    if (err != paNoError) {
        std::cerr << "Failed to open audio stream: " << Pa_GetErrorText(err) << std::endl;
        return false;
    }
    
    return true;
}

bool AudioProcessor::startMicrophone() {
    if (!stream) {
        if (!initializeMicrophone()) {
            return false;
        }
    }
    
    PaError err = Pa_StartStream(stream);
    if (err != paNoError) {
        std::cerr << "Failed to start audio stream: " << Pa_GetErrorText(err) << std::endl;
        return false;
    }
    
    microphoneActive = true;
    std::cout << "Microphone activated" << std::endl;
    return true;
}

void AudioProcessor::stopMicrophone() {
    if (stream && microphoneActive) {
        Pa_StopStream(stream);
        microphoneActive = false;
        std::cout << "Microphone deactivated" << std::endl;
    }
}

void AudioProcessor::getMicrophoneBuffer(std::vector<float>& buffer) {
    std::lock_guard<std::mutex> lock(bufferMutex);
    buffer = micBuffer;
}

int AudioProcessor::audioCallback(const void* inputBuffer, void* outputBuffer,
                                  unsigned long framesPerBuffer,
                                  const PaStreamCallbackTimeInfo* timeInfo,
                                  PaStreamCallbackFlags statusFlags,
                                  void* userData) {
    AudioProcessor* processor = static_cast<AudioProcessor*>(userData);
    const float* input = static_cast<const float*>(inputBuffer);
    
    if (input) {
        std::lock_guard<std::mutex> lock(processor->bufferMutex);
        for (unsigned long i = 0; i < framesPerBuffer && i < processor->micBuffer.size(); i++) {
            processor->micBuffer[i] = input[i];
        }
    }
    
    return paContinue;
}
