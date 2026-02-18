#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include "audio_processor.h"
#include "visualizer.h"

// Window dimensions
const int WINDOW_WIDTH = 1200;
const int WINDOW_HEIGHT = 600;

// Callback for window resize
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// Callback for key events
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    
    // Configure GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif
    
    // Create window
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, 
                                          "Music Visualizer", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    
    // Initialize GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }
    
    // Set viewport
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    
    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Initialize audio processor and visualizer
    AudioProcessor audioProcessor(44100, 2048);
    Visualizer visualizer;
    visualizer.initialize();
    
    std::cout << "Music Visualizer Started!" << std::endl;
    std::cout << "Press ESC to exit" << std::endl;
    
    float time = 0.0f;
    
    // Main render loop
    while (!glfwWindowShouldClose(window)) {
        // Process input
        glfwPollEvents();
        
        // Generate synthetic audio data
        std::vector<float> audioBuffer;
        audioProcessor.generateSyntheticAudio(audioBuffer, time);
        
        // Perform FFT
        std::vector<float> magnitudes;
        audioProcessor.performFFT(audioBuffer, magnitudes);
        
        // Update visualizer
        visualizer.updateFrequencyData(magnitudes);
        
        // Clear screen
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        // Render visualization
        visualizer.render();
        
        // Swap buffers
        glfwSwapBuffers(window);
        
        // Update time
        time += 0.02f;
    }
    
    // Cleanup
    glfwDestroyWindow(window);
    glfwTerminate();
    
    return 0;
}
