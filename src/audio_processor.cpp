#include "audio_processor.h"
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

AudioProcessor::AudioProcessor(int sampleRate, int bufferSize)
    : sampleRate(sampleRate), bufferSize(bufferSize) {
    numBins = bufferSize / 2;
}

AudioProcessor::~AudioProcessor() {
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
