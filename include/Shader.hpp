#pragma once

#include "../include/Matrix4.hpp"
#include "../include/Renderer.hpp"
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

    void setUniform4f(const std::string &name, float v0, float v1, float v2, float v3);
    void setUniformMat4f(const std::string &name, const Matrix4 &matrix);

    private:
    unsigned int _rendererId;
    unsigned int compileShader(unsigned int type, const std::string &source);
    unsigned int createShader(const std::string &vertexShader, const std::string &fragmentShader);

    std::unordered_map<std::string, int> _uniformLocationCache;
    ShaderProgramSource parseShader(const std::string &file);
    int getUniformLocation(const std::string &name);
};