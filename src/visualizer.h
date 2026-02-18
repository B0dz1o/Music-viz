#ifndef VISUALIZER_H
#define VISUALIZER_H

#include <vector>
#include <GL/glew.h>
#include "shader.h"

class Visualizer {
public:
    Visualizer();
    ~Visualizer();
    
    void initialize();
    void updateFrequencyData(const std::vector<float>& magnitudes);
    void render();
    
private:
    Shader* shader;
    GLuint VAO, VBO;
    std::vector<float> frequencies;
    int numBars;
    
    void setupBuffers();
};

#endif // VISUALIZER_H
