#ifndef VISUALIZER_H
#define VISUALIZER_H

#include <vector>
#include <memory>
#include <GL/glew.h>
#include "shader.h"

enum class VisualizationMode {
    FREQUENCY_BARS,    // Original equalizer bars
    CIRCULAR_SPECTRUM, // Circular/radial spectrum
    WAVEFORM,          // Waveform display
    CIRCLE_PULSE       // Pulsing circles
};

class Visualizer {
public:
    Visualizer();
    ~Visualizer();
    
    void initialize();
    void updateFrequencyData(const std::vector<float>& magnitudes);
    void updateWaveformData(const std::vector<float>& audioBuffer);
    void render();
    void setMode(VisualizationMode mode);
    void setBeat(float level);
    void adjustPulseScale(float delta);
    VisualizationMode getMode() const { return currentMode; }
    
private:
    std::unique_ptr<Shader> shader;
    GLuint VAO, VBO;
    std::vector<float> frequencies;
    std::vector<float> waveform;
    int numBars;
    VisualizationMode currentMode;
    float beatLevel;
    
    void setupBuffers();
    void renderFrequencyBars();
    void renderCircularSpectrum();
    void renderWaveform();
    void renderCirclePulse();
    // pulse control
    float baseInner;
    float pulseScale;
};

#endif // VISUALIZER_H
