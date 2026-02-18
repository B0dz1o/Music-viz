#include "visualizer.h"
#include <iostream>
#include <algorithm>
#include <cmath>

Visualizer::Visualizer() : VAO(0), VBO(0), numBars(64) {
    frequencies.resize(numBars, 0.0f);
}

Visualizer::~Visualizer() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

void Visualizer::initialize() {
    shader = std::make_unique<Shader>("shaders/vertex.glsl", "shaders/fragment.glsl");
    setupBuffers();
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
        
        // Apply logarithmic scaling and smoothing
        float target = std::log(1.0f + avg * 100.0f) * 0.5f;
        frequencies[i] = frequencies[i] * 0.7f + target * 0.3f;
    }
}

void Visualizer::render() {
    shader->use();
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
