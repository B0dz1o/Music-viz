#include "visualizer.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <array>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Visualizer::Visualizer() : VAO(0), VBO(0), numBars(64), currentMode(VisualizationMode::FREQUENCY_BARS), beatLevel(0.0f), particleVAO(0), particleVBO(0) {
    frequencies.resize(numBars, 0.0f);
    waveform.resize(2048, 0.0f);
    baseInner = 0.32f;
    pulseScale = 0.08f;
    particles.reserve(maxParticles);
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
}

Visualizer::~Visualizer() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &particleVAO);
    glDeleteBuffers(1, &particleVBO);
}

void Visualizer::initialize() {
    shader = std::make_unique<Shader>("shaders/vertex.glsl", "shaders/fragment.glsl");
    setupBuffers();
    setupParticleBuffers();
}

void Visualizer::setupBuffers() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    
    // Allocate buffer for maximum vertices (6 vertices per bar)
    glBufferData(GL_ARRAY_BUFFER, numBars * 6 * 2 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    
    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Visualizer::setupParticleBuffers() {
    glGenVertexArrays(1, &particleVAO);
    glGenBuffers(1, &particleVBO);

    glBindVertexArray(particleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, particleVBO);
    // Reserve space for one particle position (updated per draw call)
    glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Visualizer::updateFrequencyData(const std::vector<float>& magnitudes) {
    // Update frequency data with smoothing
    int step = magnitudes.size() / numBars;
    if (step < 1) step = 1;
    
    for (int i = 0; i < numBars && i * step < magnitudes.size(); i++) {
        float sum = 0.0f;
        int count = 0;
        
        for (int j = 0; j < step && (i * step + j) < magnitudes.size(); j++) {
            sum += magnitudes[i * step + j];
            count++;
        }
        
        float avg = (count > 0) ? sum / count : 0.0f;
        
        // Apply logarithmic scaling and smoothing with increased sensitivity
        float target = std::log(1.0f + avg * 300.0f) * 0.8f;
        frequencies[i] = frequencies[i] * 0.7f + target * 0.3f;
    }
}

void Visualizer::updateWaveformData(const std::vector<float>& audioBuffer) {
    // Store waveform data with smoothing
    for (size_t i = 0; i < std::min(audioBuffer.size(), waveform.size()); i++) {
        waveform[i] = waveform[i] * 0.5f + audioBuffer[i] * 0.5f;
    }
}

void Visualizer::setMode(VisualizationMode mode) {
    currentMode = mode;
}

void Visualizer::render() {
    shader->use();
    
    switch (currentMode) {
        case VisualizationMode::FREQUENCY_BARS:
            renderFrequencyBars();
            break;
        case VisualizationMode::CIRCULAR_SPECTRUM:
            renderCircularSpectrum();
            break;
        case VisualizationMode::WAVEFORM:
            renderWaveform();
            break;
        case VisualizationMode::CIRCLE_PULSE:
            renderCirclePulse();
            break;
        case VisualizationMode::PARTICLE_EFFECTS:
            renderParticleEffects();
            break;
    }
}

void Visualizer::renderFrequencyBars() {
    glBindVertexArray(VAO);
    
    float barWidth = 2.0f / numBars;
    std::vector<float> vertices;
    vertices.reserve(numBars * 12); // 6 vertices * 2 coordinates per bar
    
    // Batch all bar geometry into a single buffer
    for (int i = 0; i < numBars; i++) {
        float x = -1.0f + i * barWidth;
        float height = std::min(frequencies[i], 2.0f);
        
        // Create vertices for a bar (two triangles)
        vertices.push_back(x);
        vertices.push_back(0.0f);
        vertices.push_back(x + barWidth * 0.9f);
        vertices.push_back(0.0f);
        vertices.push_back(x + barWidth * 0.9f);
        vertices.push_back(height);
        
        vertices.push_back(x);
        vertices.push_back(0.0f);
        vertices.push_back(x + barWidth * 0.9f);
        vertices.push_back(height);
        vertices.push_back(x);
        vertices.push_back(height);
    }
    
    // Upload all vertices at once
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data());
    
    // Draw all bars with color variation
    for (int i = 0; i < numBars; i++) {
        float height = std::min(frequencies[i], 2.0f);
        float r = 0.2f + height * 0.4f;
        float g = 0.5f + height * 0.3f;
        float b = 0.8f - height * 0.3f;
        shader->setVec3("barColor", r, g, b);
        
        glDrawArrays(GL_TRIANGLES, i * 6, 6);
    }
    
    glBindVertexArray(0);
}

void Visualizer::renderCircularSpectrum() {
    glBindVertexArray(VAO);
    
    std::vector<float> vertices;
    vertices.reserve(numBars * 12); // 2 triangles per bar, 6 vertices each, 2 floats per vertex
    
    float angleStep = 2.0f * M_PI / numBars;
    // Query current viewport to compute aspect ratio so the circle stays round
    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);
    float vw = static_cast<float>(vp[2]);
    float vh = static_cast<float>(vp[3]);
    float aspect = (vw > 0.0f) ? (vh / vw) : 1.0f; // height/width

    // Base radius in NDC (relative to [-1,1] range). Use member fields.
    float innerRadius = baseInner + beatLevel * pulseScale;
    // Scale X coordinates by aspect so circle remains circular in pixel space
    float xScale = aspect;
    
    for (int i = 0; i < numBars; i++) {
        float angle = i * angleStep;
        float height = std::min(frequencies[i], 1.0f) * 0.5f;
        float outerRadius = innerRadius + height;
        
        float x1 = innerRadius * cos(angle) * xScale;
        float y1 = innerRadius * sin(angle);
        float x2 = outerRadius * cos(angle) * xScale;
        float y2 = outerRadius * sin(angle);
        
        float nextAngle = (i + 1) * angleStep;
        float x3 = innerRadius * cos(nextAngle) * xScale;
        float y3 = innerRadius * sin(nextAngle);
        float x4 = outerRadius * cos(nextAngle) * xScale;
        float y4 = outerRadius * sin(nextAngle);
        
        // First triangle
        vertices.push_back(x1); vertices.push_back(y1);
        vertices.push_back(x2); vertices.push_back(y2);
        vertices.push_back(x4); vertices.push_back(y4);
        
        // Second triangle
        vertices.push_back(x1); vertices.push_back(y1);
        vertices.push_back(x4); vertices.push_back(y4);
        vertices.push_back(x3); vertices.push_back(y3);
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data());
    
    // Draw with rainbow colors
    for (int i = 0; i < numBars; i++) {
        float hue = static_cast<float>(i) / numBars;
        float height = std::min(frequencies[i], 1.0f);
        float r = 0.5f + 0.5f * cos(hue * 2.0f * M_PI);
        float g = 0.5f + 0.5f * cos(hue * 2.0f * M_PI + 2.0f * M_PI / 3.0f);
        float b = 0.5f + 0.5f * cos(hue * 2.0f * M_PI + 4.0f * M_PI / 3.0f);
        
        // Brighten based on amplitude
        r = r * 0.5f + height * 0.5f;
        g = g * 0.5f + height * 0.5f;
        b = b * 0.5f + height * 0.5f;
        
        shader->setVec3("barColor", r, g, b);
        glDrawArrays(GL_TRIANGLES, i * 6, 6);
    }
    
    glBindVertexArray(0);
}

void Visualizer::renderWaveform() {
    glBindVertexArray(VAO);
    
    std::vector<float> vertices;
    int numSamples = std::min(static_cast<int>(waveform.size()), 512);
    vertices.reserve(numSamples * 2);
    
    for (int i = 0; i < numSamples; i++) {
        float x = -1.0f + (2.0f * i) / numSamples;
        float y = waveform[i * waveform.size() / numSamples] * 0.8f;
        vertices.push_back(x);
        vertices.push_back(y);
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data());
    
    // Draw waveform as a line strip
    shader->setVec3("barColor", 0.2f, 0.8f, 0.9f);
    glLineWidth(2.0f);
    glDrawArrays(GL_LINE_STRIP, 0, numSamples);
    glLineWidth(1.0f);
    
    glBindVertexArray(0);
}

void Visualizer::renderCirclePulse() {
    glBindVertexArray(VAO);
    
    // Calculate average frequency in different bands
    int bandsCount = 4;
    std::vector<float> bands(bandsCount, 0.0f);
    int barsPerBand = numBars / bandsCount;
    
    for (int b = 0; b < bandsCount; b++) {
        float sum = 0.0f;
        for (int i = 0; i < barsPerBand; i++) {
            int idx = b * barsPerBand + i;
            if (idx < numBars) {
                sum += frequencies[idx];
            }
        }
        bands[b] = sum / barsPerBand;
    }
    
    std::vector<float> vertices;
    int segments = 64;
    
    for (int b = 0; b < bandsCount; b++) {
        float radius = 0.2f + b * 0.15f + bands[b] * 0.1f;
        float angleStep = 2.0f * M_PI / segments;
        
        for (int i = 0; i < segments; i++) {
            float angle1 = i * angleStep;
            float angle2 = (i + 1) * angleStep;
            
            // Triangle from center
            vertices.push_back(0.0f); vertices.push_back(0.0f);
            vertices.push_back(radius * cos(angle1)); vertices.push_back(radius * sin(angle1));
            vertices.push_back(radius * cos(angle2)); vertices.push_back(radius * sin(angle2));
        }
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data());
    
    // Color palette for each band
    static const std::array<std::array<float, 3>, 4> colors = {{
        {1.0f, 0.2f, 0.2f},  // Red
        {0.2f, 1.0f, 0.2f},  // Green
        {0.2f, 0.2f, 1.0f},  // Blue
        {1.0f, 1.0f, 0.2f}   // Yellow
    }};
    
    // Draw each band with different colors
    for (int b = 0; b < bandsCount; b++) {
        float intensity = std::min(bands[b], 1.0f);
        shader->setVec3("barColor", 
                       colors[b][0] * (0.3f + intensity * 0.7f),
                       colors[b][1] * (0.3f + intensity * 0.7f),
                       colors[b][2] * (0.3f + intensity * 0.7f));
        glDrawArrays(GL_TRIANGLES, b * segments * 3, segments * 3);
    }
    
    glBindVertexArray(0);
}

void Visualizer::renderParticleEffects() {
    const float dt = 0.02f;   // fixed timestep matching the main render loop
    const float gravity = -0.4f;

    // Spawn particles driven by frequency bands
    const int numBands = 8;
    const int barsPerBand = numBars / numBands;

    for (int b = 0; b < numBands; b++) {
        float energy = 0.0f;
        for (int i = 0; i < barsPerBand; i++) {
            energy += frequencies[b * barsPerBand + i];
        }
        energy /= barsPerBand;
        energy = std::min(energy, 1.5f);

        int numNew = static_cast<int>(energy * 4.0f);
        for (int n = 0; n < numNew && static_cast<int>(particles.size()) < maxParticles; n++) {
            Particle p;
            float bandCenterX = -1.0f + (2.0f * (b + 0.5f)) / numBands;
            p.x = bandCenterX + (rand() % 201 - 100) / 100.0f * (1.0f / numBands);
            p.y = -1.0f;
            p.vx = (rand() % 201 - 100) / 600.0f;
            p.vy = 0.4f + energy * 0.6f + (rand() % 100) / 400.0f;
            p.maxLife = 1.2f + energy * 0.8f;
            p.life = p.maxLife;
            // Rainbow color per band
            float hue = static_cast<float>(b) / numBands;
            p.r = 0.5f + 0.5f * std::cos(hue * 2.0f * M_PI);
            p.g = 0.5f + 0.5f * std::cos(hue * 2.0f * M_PI + 2.0f * M_PI / 3.0f);
            p.b = 0.5f + 0.5f * std::cos(hue * 2.0f * M_PI + 4.0f * M_PI / 3.0f);
            p.size = 3.0f + energy * 5.0f;
            particles.push_back(p);
        }
    }

    // Spawn burst particles on a strong beat
    if (beatLevel > 0.5f) {
        int numBurst = static_cast<int>(beatLevel * 8.0f);
        for (int n = 0; n < numBurst && static_cast<int>(particles.size()) < maxParticles; n++) {
            Particle p;
            p.x = (rand() % 201 - 100) / 100.0f * 0.4f;
            p.y = (rand() % 201 - 100) / 100.0f * 0.4f;
            float angle = (rand() % 360) * M_PI / 180.0f;
            float speed = 0.3f + beatLevel * 0.6f;
            p.vx = std::cos(angle) * speed;
            p.vy = std::sin(angle) * speed;
            p.maxLife = 0.8f + beatLevel * 0.4f;
            p.life = p.maxLife;
            p.r = 1.0f; p.g = 0.85f; p.b = 0.2f; // bright gold burst
            p.size = 4.0f + beatLevel * 6.0f;
            particles.push_back(p);
        }
    }

    // Update all particles
    for (auto& p : particles) {
        p.vy += gravity * dt;
        p.x += p.vx * dt;
        p.y += p.vy * dt;
        p.life -= dt;
    }

    // Remove dead or out-of-bounds particles
    particles.erase(
        std::remove_if(particles.begin(), particles.end(), [](const Particle& p) {
            return p.life <= 0.0f || p.y < -1.1f || p.x < -1.2f || p.x > 1.2f;
        }),
        particles.end()
    );

    // Render each particle as a point
    glBindVertexArray(particleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, particleVBO);

    for (const auto& p : particles) {
        float alpha = p.life / p.maxLife;
        float pos[2] = {p.x, p.y};
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(pos), pos);
        shader->setVec3("barColor", p.r * alpha, p.g * alpha, p.b * alpha);
        glPointSize(std::max(1.0f, p.size * alpha));
        glDrawArrays(GL_POINTS, 0, 1);
    }

    glPointSize(1.0f);
    glBindVertexArray(0);
}

void Visualizer::setBeat(float level) {
    // clamp
    if (level < 0.0f) level = 0.0f;
    if (level > 1.0f) level = 1.0f;
    beatLevel = level;
}

void Visualizer::adjustPulseScale(float delta) {
    pulseScale += delta;
    if (pulseScale < 0.01f) pulseScale = 0.01f;
    if (pulseScale > 0.5f) pulseScale = 0.5f;
    std::cout << "Pulse scale = " << pulseScale << std::endl;
}
