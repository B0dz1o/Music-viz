#include "particle_system.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <cstring>

// Layout of one particle in the VBO (interleaved, 6 floats = 24 bytes):
//  [0] pos.x  [1] pos.y  [2] vel.x  [3] vel.y  [4] life  [5] size
static constexpr int FLOATS_PER_PARTICLE = 6;
static constexpr int PARTICLE_STRIDE     = FLOATS_PER_PARTICLE * sizeof(float);

ParticleSystem::ParticleSystem(int n)
    : numParticles(n), readIdx(0),
      transformFeedback(0),
      updateProgram(0), renderProgram(0),
      uUpdate_DeltaTime(-1), uUpdate_Time(-1),
      uUpdate_BeatLevel(-1), uUpdate_BassEnergy(-1),
      uUpdate_MidEnergy(-1), uUpdate_HighEnergy(-1),
      uRender_BeatLevel(-1), uRender_Time(-1),
      uRender_BassEnergy(-1), uRender_HighEnergy(-1)
{
    VAO[0] = VAO[1] = 0;
    VBO[0] = VBO[1] = 0;
}

ParticleSystem::~ParticleSystem() {
    glDeleteVertexArrays(2, VAO);
    glDeleteBuffers(2, VBO);
    glDeleteTransformFeedbacks(1, &transformFeedback);
    if (updateProgram) glDeleteProgram(updateProgram);
    if (renderProgram) glDeleteProgram(renderProgram);
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

void ParticleSystem::initialize() {
    // --- Initialize particle data (all dead so they respawn immediately) ---
    std::vector<float> initial(numParticles * FLOATS_PER_PARTICLE, 0.0f);
    // life = 0 means "dead → will respawn on first update"

    glGenBuffers(2, VBO);
    for (int i = 0; i < 2; ++i) {
        glBindBuffer(GL_ARRAY_BUFFER, VBO[i]);
        glBufferData(GL_ARRAY_BUFFER,
                     numParticles * PARTICLE_STRIDE,
                     initial.data(),
                     GL_DYNAMIC_COPY);
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // --- VAOs ---
    glGenVertexArrays(2, VAO);
    setupVAO(0);
    setupVAO(1);

    // --- Transform feedback object ---
    glGenTransformFeedbacks(1, &transformFeedback);

    // --- Shader programs ---
    buildUpdateProgram();
    buildRenderProgram();
}

void ParticleSystem::update(float dt, float beatLevel,
                             float bassEnergy, float midEnergy, float highEnergy,
                             float time)
{
    int writeIdx = 1 - readIdx;

    glUseProgram(updateProgram);
    glUniform1f(uUpdate_DeltaTime,  dt);
    glUniform1f(uUpdate_Time,       time);
    glUniform1f(uUpdate_BeatLevel,  beatLevel);
    glUniform1f(uUpdate_BassEnergy, bassEnergy);
    glUniform1f(uUpdate_MidEnergy,  midEnergy);
    glUniform1f(uUpdate_HighEnergy, highEnergy);

    // Bind TFO and output buffer
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, transformFeedback);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, VBO[writeIdx]);

    // Suppress rasterization — this pass is purely compute
    glEnable(GL_RASTERIZER_DISCARD);

    glBindVertexArray(VAO[readIdx]);
    glBeginTransformFeedback(GL_POINTS);
    glDrawArrays(GL_POINTS, 0, numParticles);
    glEndTransformFeedback();

    glDisable(GL_RASTERIZER_DISCARD);
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
    glBindVertexArray(0);

    // Swap buffers
    readIdx = writeIdx;
}

void ParticleSystem::render(float beatLevel, float bassEnergy, float highEnergy, float time) {
    // Additive blending makes overlapping particles glow
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    glEnable(GL_PROGRAM_POINT_SIZE);

    glUseProgram(renderProgram);
    glUniform1f(uRender_BeatLevel,  beatLevel);
    glUniform1f(uRender_Time,       time);
    glUniform1f(uRender_BassEnergy, bassEnergy);
    glUniform1f(uRender_HighEnergy, highEnergy);

    glBindVertexArray(VAO[readIdx]);
    glDrawArrays(GL_POINTS, 0, numParticles);
    glBindVertexArray(0);

    glDisable(GL_PROGRAM_POINT_SIZE);

    // Restore default blending
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

void ParticleSystem::setupVAO(int idx) {
    glBindVertexArray(VAO[idx]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[idx]);

    // location 0: position (vec2)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, PARTICLE_STRIDE,
                          reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(0);

    // location 1: velocity (vec2)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, PARTICLE_STRIDE,
                          reinterpret_cast<void*>(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // location 2: life (float)
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, PARTICLE_STRIDE,
                          reinterpret_cast<void*>(4 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // location 3: size (float)
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, PARTICLE_STRIDE,
                          reinterpret_cast<void*>(5 * sizeof(float)));
    glEnableVertexAttribArray(3);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void ParticleSystem::buildUpdateProgram() {
    GLuint vert = compileShader("shaders/particle_update.vert", GL_VERTEX_SHADER);
    GLuint frag = compileShader("shaders/particle_update.frag", GL_FRAGMENT_SHADER);

    updateProgram = glCreateProgram();
    glAttachShader(updateProgram, vert);
    glAttachShader(updateProgram, frag);

    // Specify which vertex-shader outputs to capture (interleaved into one buffer)
    const char* varyings[] = { "outPosition", "outVelocity", "outLife", "outSize" };
    glTransformFeedbackVaryings(updateProgram, 4, varyings, GL_INTERLEAVED_ATTRIBS);

    glLinkProgram(updateProgram);

    GLint ok = 0;
    glGetProgramiv(updateProgram, GL_LINK_STATUS, &ok);
    if (!ok) {
        GLint len = 0;
        glGetProgramiv(updateProgram, GL_INFO_LOG_LENGTH, &len);
        std::vector<char> log(len);
        glGetProgramInfoLog(updateProgram, len, nullptr, log.data());
        std::cerr << "particle_update program link error:\n" << log.data() << std::endl;
    }

    glDeleteShader(vert);
    glDeleteShader(frag);

    // Cache uniform locations
    uUpdate_DeltaTime  = glGetUniformLocation(updateProgram, "uDeltaTime");
    uUpdate_Time       = glGetUniformLocation(updateProgram, "uTime");
    uUpdate_BeatLevel  = glGetUniformLocation(updateProgram, "uBeatLevel");
    uUpdate_BassEnergy = glGetUniformLocation(updateProgram, "uBassEnergy");
    uUpdate_MidEnergy  = glGetUniformLocation(updateProgram, "uMidEnergy");
    uUpdate_HighEnergy = glGetUniformLocation(updateProgram, "uHighEnergy");
}

void ParticleSystem::buildRenderProgram() {
    GLuint vert = compileShader("shaders/particle_render.vert", GL_VERTEX_SHADER);
    GLuint frag = compileShader("shaders/particle_render.frag", GL_FRAGMENT_SHADER);

    renderProgram = glCreateProgram();
    glAttachShader(renderProgram, vert);
    glAttachShader(renderProgram, frag);
    glLinkProgram(renderProgram);

    GLint ok = 0;
    glGetProgramiv(renderProgram, GL_LINK_STATUS, &ok);
    if (!ok) {
        GLint len = 0;
        glGetProgramiv(renderProgram, GL_INFO_LOG_LENGTH, &len);
        std::vector<char> log(len);
        glGetProgramInfoLog(renderProgram, len, nullptr, log.data());
        std::cerr << "particle_render program link error:\n" << log.data() << std::endl;
    }

    glDeleteShader(vert);
    glDeleteShader(frag);

    // Cache uniform locations
    uRender_BeatLevel  = glGetUniformLocation(renderProgram, "uBeatLevel");
    uRender_Time       = glGetUniformLocation(renderProgram, "uTime");
    uRender_BassEnergy = glGetUniformLocation(renderProgram, "uBassEnergy");
    uRender_HighEnergy = glGetUniformLocation(renderProgram, "uHighEnergy");
}

GLuint ParticleSystem::compileShader(const std::string& path, GLenum type) {
    std::string src = readFile(path);
    const char* csrc = src.c_str();

    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &csrc, nullptr);
    glCompileShader(shader);

    GLint ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        GLint len = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
        std::vector<char> log(len);
        glGetShaderInfoLog(shader, len, nullptr, log.data());
        std::cerr << "Shader compile error (" << path << "):\n" << log.data() << std::endl;
    }
    return shader;
}

std::string ParticleSystem::readFile(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) {
        std::cerr << "ParticleSystem: cannot open " << path << std::endl;
        return "";
    }
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}
