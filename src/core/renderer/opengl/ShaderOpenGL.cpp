#include "renderer/opengl/ShaderOpenGL.hpp"
#include "renderer/opengl/RendererOpenGL.hpp"
#include <alloca.h>
#include <fstream>
#include <sstream>
#include <string>

ShaderOpenGL::ShaderOpenGL(const std::string& filename) : _rendererId(0)
{
    ShaderProgramSource source = ParseShader(filename);
    _rendererId = CreateShader(source.vertexSource, source.fragmentSource);
}
ShaderOpenGL::~ShaderOpenGL()
{
    GlCall(glDeleteProgram(_rendererId));
}

unsigned int ShaderOpenGL::CompileShader(unsigned int type, const std::string& source)
{
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    GlCall(glShaderSource(id, 1, &src, nullptr));
    GlCall(glCompileShader(id));

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE)
    {
        int length;
        GlCall(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
        char* message = (char*)alloca(length * sizeof(char));
        GlCall(glGetShaderInfoLog(id, length, &length, message));
        std::cerr << "Failed to compile shader." << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << "\n";
        std::cerr << message << "\n";
        GlCall(glDeleteShader(id));
        return 0;
    }
    return id;
}

unsigned int ShaderOpenGL::CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
{
    unsigned int program = glCreateProgram();

    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    GlCall(glAttachShader(program, vs));
    GlCall(glAttachShader(program, fs));
    GlCall(glLinkProgram(program));
    GlCall(glValidateProgram(program));

    GlCall(glDeleteShader(vs));
    GlCall(glDeleteShader(fs));

    return program;
}

ShaderOpenGL::ShaderProgramSource ShaderOpenGL::ParseShader(const std::string& file)
{
    std::ifstream stream(file);

    std::string line;
    std::stringstream ss[2];

    ShaderType type = ShaderType::NONE;

    while (std::getline(stream, line))
    {

        if (line.find("#shader") != std::string::npos)
        {
            if (line.find("vertex") != std::string::npos)
            {
                type = ShaderType::VERTEX;
            }
            else if (line.find("fragment") != std::string::npos)
            {
                type = ShaderType::FRAGMENT;
            }
        }
        else
        {
            ss[(int)type] << line << "\n";
        }
    }

    return {ss[0].str(), ss[1].str()};
}

void ShaderOpenGL::Bind() const
{
    GlCall(glUseProgram(_rendererId));
}
void ShaderOpenGL::Unbind() const
{
    GlCall(glUseProgram(0));
}

void ShaderOpenGL::SetUniformMat4f(const std::string& name, const Matrix4& matrix)
{
    GlCall(glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &matrix._m[0][0]));
}

void ShaderOpenGL::SetUniform1i(const std::string& name, float value)
{
    GlCall(glUniform1i(GetUniformLocation(name), value));
}

void ShaderOpenGL::SetUniform1f(const std::string& name, float value)
{
    GlCall(glUniform1f(GetUniformLocation(name), value));
}

int ShaderOpenGL::GetUniformLocation(const std::string& name)
{
    if (_uniformCache.find(name) != _uniformCache.end())
    {
        return _uniformCache[name];
    }

    int location = glGetUniformLocation(_rendererId, name.c_str());
    if (location == -1)
    {
        // Not having a uniform is not critical, so no need to throw an exception.
        // Model will still be displayed.
        std::cerr << "Warning: uniform '" << name << "' doesn't exist!" << "\n";
    }
    _uniformCache[name] = location;
    return location;
}
