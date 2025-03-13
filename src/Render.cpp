#include "../include/Render.hpp"
#include "../include/colors.hpp"
#include "SDL3/SDL_error.h"
#include "SDL3/SDL_init.h"
#include <alloca.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

Render::Render(const std::string &file)
    : _window(nullptr), _gl_context(nullptr), _running(true), _vao(0), _vbo(0), _ebo(0) {
    _model.parse(file);
}

Render::~Render() {

    SDL_DestroyWindow(_window);
    SDL_GL_DestroyContext(_gl_context);
    SDL_Quit();
}

void Render::init() {
    if (!SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO)) {
        throw std::runtime_error(SDL_GetError());
    }

    // https://stackoverflow.com/questions/19865463/opengl-4-1-under-mavericks
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    _window = SDL_CreateWindow(W_TITLE, W_WIDTH, W_HEIGHT, SDL_WINDOW_OPENGL);

    if (!_window) {
        throw std::runtime_error(SDL_GetError());
    }
    _gl_context = SDL_GL_CreateContext(_window);

    if (!_gl_context) {
        throw std::runtime_error(SDL_GetError());
    }

    SDL_GL_MakeCurrent(_window, _gl_context);
    SDL_GL_SetSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        throw std::runtime_error(SDL_GetError());
    }

    std::cout << BOLD_ITALIC_CYAN << "Using Renderer: " << glGetString(GL_RENDERER) << " " << glGetString(GL_VERSION)
              << RESET << std::endl;
}

unsigned int Render::compileShader(unsigned int type, const std::string &source) {
    unsigned int id = glCreateShader(type);
    const char *src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char *message = (char *)alloca(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length, message);
        std::cerr << "Failed to compile shader." << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << std::endl;
        std::cerr << message << std::endl;
        glDeleteShader(id);
        return 0;
    }
    return id;
}

int Render::createShader(const std::string &vertexShader, const std::string &fragmentShader) {
    unsigned int program = glCreateProgram();

    unsigned int vs = compileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fragmentShader);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    _modelMatrixLocation = glGetUniformLocation(program, "modelMatrix");           // TODO: Check error
    _projectionMatrixLocation = glGetUniformLocation(program, "projectionMatrix"); // TODO: Check error

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

Render::ShaderProgramSource Render::parseShader(const std::string &file) {
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

void Render::mainLoop() {
    // clang-format off
    // float position[] = {
    //     -0.5,  0.5, 0.0,
    //      0.5,  0.5, 0.0,
    //      0.5, -0.5, 0.0,
    //     -0.5, -0.5, 0.0,
    // };
    // GLuint indices[] = {0, 1, 2, 0, 2, 3};
    // clang-format on

    std::vector<unsigned int> indices;
    indices.reserve(_model.getFaces().size() * 3);

    for (auto &face : _model.getFaces()) {
        indices.push_back(face.getVertexIndices()[0]);
        indices.push_back(face.getVertexIndices()[1]);
        indices.push_back(face.getVertexIndices()[2]);
    }

    glGenVertexArrays(1, &_vao);
    glGenBuffers(1, &_vbo);
    glGenBuffers(1, &_ebo);

    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);

    // Choose Static or Dynamic depending on situation. It can effect performance.
    glBufferData(GL_ARRAY_BUFFER, _model.getVertex().size() * sizeof(Vector3), _model.getVertex().data(),
                 GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    // Stride -> Number of bytes to get to next vertex.
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3), 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    ShaderProgramSource source = parseShader("../res/Basic.glsl");

    unsigned int shader = createShader(source.vertexSource, source.fragmentSource);
    glUseProgram(shader);

    Vector3 position(0.0, 0.0, -5.0);

    _projectionMatrix = Matrix4::perspective(70.0, W_WIDTH, W_HEIGHT, 0.01, 10000.0);
    glUniformMatrix4fv(_projectionMatrixLocation, 1, GL_FALSE, (float *)&_projectionMatrix);

    while (_running) {
        _modelMatrix = Matrix4::translation(position);
        glUniformMatrix4fv(_modelMatrixLocation, 1, GL_FALSE, (float *)&_modelMatrix);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        // glCullFace(GL_BACK);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);

        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED || event.type == SDL_EVENT_QUIT) {
                _running = false;
            }
        }

        SDL_GL_SwapWindow(_window);
    }
}