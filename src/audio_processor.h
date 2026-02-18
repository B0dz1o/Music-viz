#ifndef AUDIO_PROCESSOR_H
#define AUDIO_PROCESSOR_H

#include <vector>
#include <cmath>
#include <portaudio.h>
#include <mutex>

class AudioProcessor {
public:
    AudioProcessor(int sampleRate = 44100, int bufferSize = 2048);
    ~AudioProcessor();
    
    // Generate synthetic audio data (sine waves for demo)
    void generateSyntheticAudio(std::vector<float>& buffer, float time);
    
    // Microphone input control
    bool initializeMicrophone();
    bool startMicrophone();
    void stopMicrophone();
    bool isMicrophoneActive() const { return microphoneActive; }
    void getMicrophoneBuffer(std::vector<float>& buffer);
    
    // Perform FFT on audio data
    void performFFT(const std::vector<float>& input, std::vector<float>& magnitudes);
    
    // Get frequency bins
    int getNumBins() const { return numBins; }
    
private:
    int sampleRate;
    int bufferSize;
    int numBins;
    
    // PortAudio state
    PaStream* stream;
    bool microphoneActive;
    bool microphoneInitialized;
    std::vector<float> micBuffer;
    std::mutex bufferMutex;
    
    // Simple FFT implementation
    void fft(std::vector<float>& real, std::vector<float>& imag);
    
    // PortAudio callback
    static int audioCallback(const void* inputBuffer, void* outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void* userData);
};

#endif // AUDIO_PROCESSOR_H
