#include "../include/Shader.hpp"
#include "../include/Renderer.hpp"
#include <alloca.h>
#include <fstream>
#include <sstream>
#include <string>

Shader::Shader(const std::string &filename) : _rendererId(0) {
    ShaderProgramSource source = parseShader(filename);
    _rendererId = createShader(source.vertexSource, source.fragmentSource);
}
Shader::~Shader() {
    GlCall(glDeleteProgram(_rendererId));
}

unsigned int Shader::compileShader(unsigned int type, const std::string &source) {
    unsigned int id = glCreateShader(type);
    const char *src = source.c_str();
    GlCall(glShaderSource(id, 1, &src, nullptr));
    GlCall(glCompileShader(id));

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        int length;
        GlCall(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
        char *message = (char *)alloca(length * sizeof(char));
        GlCall(glGetShaderInfoLog(id, length, &length, message));
        std::cerr << "Failed to compile shader." << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << std::endl;
        std::cerr << message << std::endl;
        GlCall(glDeleteShader(id));
        return 0;
    }
    return id;
}

unsigned int Shader::createShader(const std::string &vertexShader, const std::string &fragmentShader) {
    unsigned int program = glCreateProgram();

    unsigned int vs = compileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fragmentShader);

    GlCall(glAttachShader(program, vs));
    GlCall(glAttachShader(program, fs));
    GlCall(glLinkProgram(program));
    GlCall(glValidateProgram(program));

    GlCall(glDeleteShader(vs));
    GlCall(glDeleteShader(fs));

    return program;
}

Shader::ShaderProgramSource Shader::parseShader(const std::string &file) {
    std::ifstream stream(file);

    std::string line;
    std::stringstream ss[2];

    ShaderType type = ShaderType::NONE;

    while (std::getline(stream, line)) {

        if (line.find("#shader") != std::string::npos) {
            if (line.find("vertex") != std::string::npos) {
                type = ShaderType::VERTEX;
            } else if (line.find("fragment") != std::string::npos) {
                type = ShaderType::FRAGMENT;
            }
        } else {
            ss[(int)type] << line << "\n";
        }
    }

    return {ss[0].str(), ss[1].str()};
}

void Shader::Bind() const {
    GlCall(glUseProgram(_rendererId));
}
void Shader::Unbind() const {
    GlCall(glUseProgram(0));
}
int Shader::getUniformLocation(const std::string &name) {
    if (_uniformLocationCache.find(name) != _uniformLocationCache.end()) {
        return _uniformLocationCache[name];
    }

    int location = glGetUniformLocation(_rendererId, name.c_str());
    if (location == -1) {
        std::cerr << "Warning: uniform '" << name << "' doesn't exist!" << std::endl;
    }
    _uniformLocationCache[name] = location;
    return location;
}
void Shader::setUniform4f(const std::string &name, float v0, float v1, float v2, float v3) {
    GlCall(glUniform4f(getUniformLocation(name), v0, v1, v2, v3));
}

void Shader::setUniformMat4f(const std::string &name, const Matrix4 &matrix) {
    GlCall(glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, &matrix._m[0][0]));
}
