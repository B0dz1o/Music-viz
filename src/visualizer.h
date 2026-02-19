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
    CIRCLE_PULSE,      // Pulsing circles
    PARTICLE_EFFECTS   // Particle system driven by audio
};

struct Particle {
    float x, y;      // position
    float vx, vy;    // velocity
    float life;      // remaining lifetime
    float maxLife;   // initial lifetime
    float r, g, b;   // color
    float size;      // point size in pixels
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

    // particle system
    static constexpr int maxParticles = 300;
    std::vector<Particle> particles;
    GLuint particleVAO, particleVBO;
    void setupParticleBuffers();
    void renderParticleEffects();
};

#endif // VISUALIZER_H
