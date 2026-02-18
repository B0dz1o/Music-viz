#include "visualizer.h"
#include <iostream>
#include <algorithm>
#include <cmath>

Visualizer::Visualizer() : shader(nullptr), VAO(0), VBO(0), numBars(64) {
    frequencies.resize(numBars, 0.0f);
}

Visualizer::~Visualizer() {
    if (shader) delete shader;
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

void Visualizer::initialize() {
    shader = new Shader("shaders/vertex.glsl", "shaders/fragment.glsl");
    setupBuffers();
}

void Visualizer::setupBuffers() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    
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
    
    for (int i = 0; i < numBars; i++) {
        float x = -1.0f + i * barWidth;
        float height = std::min(frequencies[i], 2.0f);
        
        // Create vertices for a bar
        float vertices[] = {
            x, 0.0f,
            x + barWidth * 0.9f, 0.0f,
            x + barWidth * 0.9f, height,
            x, 0.0f,
            x + barWidth * 0.9f, height,
            x, height
        };
        
        // Set color based on height
        float r = 0.2f + height * 0.4f;
        float g = 0.5f + height * 0.3f;
        float b = 0.8f - height * 0.3f;
        shader->setVec3("barColor", r, g, b);
        
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    
    glBindVertexArray(0);
}
