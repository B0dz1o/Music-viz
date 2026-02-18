#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <unordered_map>
#include <GL/glew.h>

class Shader {
public:
    GLuint programID;
    
    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    ~Shader();
    
    void use();
    void setFloat(const std::string& name, float value);
    void setVec3(const std::string& name, float x, float y, float z);
    void setMat4(const std::string& name, const float* value);
    
private:
    std::unordered_map<std::string, GLint> uniformLocations;
    
    GLuint compileShader(const std::string& source, GLenum type);
    std::string readFile(const std::string& path);
    GLint getUniformLocation(const std::string& name);
};

#endif // SHADER_H
