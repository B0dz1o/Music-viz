#ifndef AUDIO_PROCESSOR_H
#define AUDIO_PROCESSOR_H

#include <vector>
#include <cmath>

class AudioProcessor {
public:
    AudioProcessor(int sampleRate = 44100, int bufferSize = 2048);
    ~AudioProcessor();
    
    // Generate synthetic audio data (sine waves for demo)
    void generateSyntheticAudio(std::vector<float>& buffer, float time);
    
    // Perform FFT on audio data
    void performFFT(const std::vector<float>& input, std::vector<float>& magnitudes);
    
    // Get frequency bins
    int getNumBins() const { return numBins; }
    
private:
    int sampleRate;
    int bufferSize;
    int numBins;
    
    // Simple FFT implementation
    void fft(std::vector<float>& real, std::vector<float>& imag);
};

#endif // AUDIO_PROCESSOR_H
