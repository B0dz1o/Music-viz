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
    
    // Get visualizer from window user pointer
    Visualizer* visualizer = static_cast<Visualizer*>(glfwGetWindowUserPointer(window));
    if (visualizer && action == GLFW_PRESS) {
        if (key == GLFW_KEY_1) {
            visualizer->setMode(VisualizationMode::FREQUENCY_BARS);
            std::cout << "Switched to: Frequency Bars" << std::endl;
        }
        else if (key == GLFW_KEY_2) {
            visualizer->setMode(VisualizationMode::CIRCULAR_SPECTRUM);
            std::cout << "Switched to: Circular Spectrum" << std::endl;
        }
        else if (key == GLFW_KEY_3) {
            visualizer->setMode(VisualizationMode::WAVEFORM);
            std::cout << "Switched to: Waveform" << std::endl;
        }
        else if (key == GLFW_KEY_4) {
            visualizer->setMode(VisualizationMode::CIRCLE_PULSE);
            std::cout << "Switched to: Circle Pulse" << std::endl;
        }
        else if (key == GLFW_KEY_5) {
            visualizer->setMode(VisualizationMode::PARTICLES);
            std::cout << "Switched to: Particle Effects" << std::endl;
        }
    }
}

// Callback for microphone toggle (stored separately to access audio processor)
struct AppContext {
    AudioProcessor* audioProcessor;
    Visualizer* visualizer;
    bool useMicrophone;
};

void key_callback_with_audio(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    
    AppContext* context = static_cast<AppContext*>(glfwGetWindowUserPointer(window));
    if (!context || action != GLFW_PRESS) return;
    
    Visualizer* visualizer = context->visualizer;
    AudioProcessor* audioProcessor = context->audioProcessor;
    
    if (key == GLFW_KEY_1) {
        visualizer->setMode(VisualizationMode::FREQUENCY_BARS);
        std::cout << "Switched to: Frequency Bars" << std::endl;
    }
    else if (key == GLFW_KEY_2) {
        visualizer->setMode(VisualizationMode::CIRCULAR_SPECTRUM);
        std::cout << "Switched to: Circular Spectrum" << std::endl;
    }
    else if (key == GLFW_KEY_3) {
        visualizer->setMode(VisualizationMode::WAVEFORM);
        std::cout << "Switched to: Waveform" << std::endl;
    }
    else if (key == GLFW_KEY_4) {
        visualizer->setMode(VisualizationMode::CIRCLE_PULSE);
        std::cout << "Switched to: Circle Pulse" << std::endl;
    }
    else if (key == GLFW_KEY_5) {
        visualizer->setMode(VisualizationMode::PARTICLES);
        std::cout << "Switched to: Particle Effects" << std::endl;
    }
    else if (key == GLFW_KEY_M) {
        context->useMicrophone = !context->useMicrophone;
        if (context->useMicrophone) {
            if (audioProcessor->startMicrophone()) {
                std::cout << "Audio source: MICROPHONE" << std::endl;
            } else {
                std::cout << "Failed to start microphone, using synthetic audio" << std::endl;
                context->useMicrophone = false;
            }
        } else {
            audioProcessor->stopMicrophone();
            std::cout << "Audio source: SYNTHETIC" << std::endl;
        }
    }
    else if (key == GLFW_KEY_U) {
        audioProcessor->adjustSensitivity(-0.05f); // lower factor -> more sensitive
    }
    else if (key == GLFW_KEY_J) {
        audioProcessor->adjustSensitivity(0.05f); // increase factor -> less sensitive
    }
    else if (key == GLFW_KEY_I) {
        visualizer->adjustPulseScale(0.01f);
    }
    else if (key == GLFW_KEY_K) {
        visualizer->adjustPulseScale(-0.01f);
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
    
    // Initialize GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }
    
    // Set viewport using actual framebuffer size (fixes retina / initial centering)
    int fbw, fbh;
    glfwGetFramebufferSize(window, &fbw, &fbh);
    glViewport(0, 0, fbw, fbh);
    
    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Initialize audio processor and visualizer
    AudioProcessor audioProcessor(44100, 2048);
    Visualizer visualizer;
    visualizer.initialize();
    
    // Set up app context for key callback
    AppContext appContext;
    appContext.audioProcessor = &audioProcessor;
    appContext.visualizer = &visualizer;
    appContext.useMicrophone = false;
    
    glfwSetWindowUserPointer(window, &appContext);
    glfwSetKeyCallback(window, key_callback_with_audio);
    
    std::cout << "Music Visualizer Started!" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  Press 1 - Frequency Bars" << std::endl;
    std::cout << "  Press 2 - Circular Spectrum" << std::endl;
    std::cout << "  Press 3 - Waveform" << std::endl;
    std::cout << "  Press 4 - Circle Pulse" << std::endl;
    std::cout << "  Press 5 - Particle Effects (GPU)" << std::endl;
    std::cout << "  Press M - Toggle Microphone/Synthetic Audio" << std::endl;
    std::cout << "  Press ESC - Exit" << std::endl;
    std::cout << std::endl;
    std::cout << "Audio source: SYNTHETIC" << std::endl;
    
    float time = 0.0f;
    
    // Main render loop
    while (!glfwWindowShouldClose(window)) {
        // Process input
        glfwPollEvents();
        
        // Generate or capture audio data
        std::vector<float> audioBuffer;
        if (appContext.useMicrophone && audioProcessor.isMicrophoneActive()) {
            audioProcessor.getMicrophoneBuffer(audioBuffer);
        } else {
            audioProcessor.generateSyntheticAudio(audioBuffer, time);
        }
        
        // Perform FFT
        std::vector<float> magnitudes;
        audioProcessor.performFFT(audioBuffer, magnitudes);

        // Process beat detection (use same timestep as synthetic time step)
        const float dt = 0.02f;
        audioProcessor.processBeat(magnitudes, dt);
        float beatLevel = audioProcessor.getBeatLevel();
        visualizer.setBeat(beatLevel);
        
        // Update visualizer with both frequency and waveform data
        visualizer.updateFrequencyData(magnitudes);
        visualizer.updateWaveformData(audioBuffer);
        
        // Clear screen
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        // Render visualization
        visualizer.render(dt);
        
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
