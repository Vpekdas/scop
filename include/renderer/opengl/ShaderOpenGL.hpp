#pragma once

#include "math/Matrix4.hpp"
#include <cstdint>
#include <unordered_map>

class ShaderOpenGL
{
    public:
    enum class ShaderType
    {
        NONE = -1,
        VERTEX = 0,
        FRAGMENT = 1
    };

    struct ShaderProgramSource
    {
        std::string vertexSource;
        std::string fragmentSource;
    };

    ShaderOpenGL(const std::string& filename);
    ~ShaderOpenGL();

    void Bind() const;
    void Unbind() const;

    void SetUniform1i(const std::string& name, float value);
    void SetUniform1f(const std::string& name, float value);
    void SetUniformMat4f(const std::string& name, const Matrix4& matrix);

    private:
    ShaderProgramSource ParseShader(const std::string& file);
    uint32_t CompileShader(unsigned int type, const std::string& source);
    uint32_t CreateShader(const std::string& vertexShader, const std::string& fragmentShader);

    int GetUniformLocation(const std::string& name);

    uint32_t _rendererId;
    std::unordered_map<std::string, int> _uniformCache;
};