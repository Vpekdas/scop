#pragma once

#include "../include/Matrix4.hpp"
#include <iostream>
#include <unordered_map>

class Shader {
    public:
    enum class ShaderType {
        NONE = -1,
        VERTEX = 0,
        FRAGMENT = 1
    };

    struct ShaderProgramSource {
        std::string vertexSource;
        std::string fragmentSource;
    };

    Shader(const std::string &filename);
    ~Shader();

    void Bind() const;
    void Unbind() const;

    // Set texture uniform in the shader program.
    void setUniform1i(const std::string &name, float value);

    // Set Vec4 uniform in the shader program.
    void setUniform4f(const std::string &name, float v0, float v1, float v2, float v3);

    // Set a 4x4 matrix uniform in the shader program.
    void setUniformMat4f(const std::string &name, const Matrix4 &matrix);

    private:
    // Parse a single GLSL file and return two strings: one for the vs and one for the fs.
    ShaderProgramSource parseShader(const std::string &file);
    unsigned int compileShader(unsigned int type, const std::string &source);

    // Compile the vertex and fragment shaders, attach them to the program, link the program, and delete the shaders.
    unsigned int createShader(const std::string &vertexShader, const std::string &fragmentShader);

    // Return the location of a uniform variable, checking if it is present in the cache.
    int getUniformLocation(const std::string &name);
    unsigned int _rendererId;
    std::unordered_map<std::string, int> m_uniformCache;
};