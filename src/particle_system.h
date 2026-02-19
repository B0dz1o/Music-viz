#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM_H

#include <GL/glew.h>
#include <string>

// GPU-computed particle system using OpenGL Transform Feedback.
// Each frame the update pass runs the particle physics entirely on the GPU
// (ping-pong buffers), and the render pass draws GL_POINTS with soft circles.
class ParticleSystem {
public:
    explicit ParticleSystem(int numParticles = 5000);
    ~ParticleSystem();

    // Must be called after an OpenGL context is current.
    void initialize();

    // Run the GPU physics update (transform feedback).
    void update(float dt, float beatLevel,
                float bassEnergy, float midEnergy, float highEnergy,
                float time);

    // Draw particles (uses additive blending internally).
    void render(float beatLevel, float bassEnergy, float highEnergy, float time);

private:
    int   numParticles;

    // Ping-pong: VAO[i] / VBO[i] — we alternate read/write each frame.
    GLuint VAO[2];
    GLuint VBO[2];
    int    readIdx;   // index of buffer holding current particle state

    GLuint transformFeedback;   // TFO

    GLuint updateProgram;       // vertex-only (transform feedback) program
    GLuint renderProgram;       // vertex + fragment program for drawing points

    // Cached uniform locations for updateProgram
    GLint uUpdate_DeltaTime;
    GLint uUpdate_Time;
    GLint uUpdate_BeatLevel;
    GLint uUpdate_BassEnergy;
    GLint uUpdate_MidEnergy;
    GLint uUpdate_HighEnergy;

    // Cached uniform locations for renderProgram
    GLint uRender_BeatLevel;
    GLint uRender_Time;
    GLint uRender_BassEnergy;
    GLint uRender_HighEnergy;

    // Helpers
    GLuint compileShader(const std::string& path, GLenum type);
    void   buildUpdateProgram();
    void   buildRenderProgram();
    void   setupVAO(int idx);
    std::string readFile(const std::string& path);
};

#endif // PARTICLE_SYSTEM_H
