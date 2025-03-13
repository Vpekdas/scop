#include "../include/Render.hpp"
#include "../include/colors.hpp"
#include "SDL3/SDL_error.h"
#include "SDL3/SDL_init.h"
#include <alloca.h>

Render::Render() : _window(nullptr), _gl_context(nullptr), _running(true), _vao(0), _vbo(0) {
    std::cout << YELLOW << "🛠️ Default Render Constructor called 🛠️" << RESET << std::endl;
}

Render::~Render() {

    SDL_DestroyWindow(_window);
    SDL_GL_DestroyContext(_gl_context);
    SDL_Quit();
    std::cout << RED << "🧨 Render Destructor called 🧨" << RESET << std::endl;
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

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

void Render::mainLoop() {

    float position[6] = {-0.5f, -0.5f, 0.0f, 0.5f, 0.5f, -0.5f};

    glGenVertexArrays(1, &_vao);
    glGenBuffers(1, &_vbo);

    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);

    // Choose Static or Dynamic depending on situation. It can effect performance.
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), position, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    // Stride -> Number of bytes to get to next vertex.
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    const std::string vertexShader = "#version 330 core\n"
                                     "\n"
                                     "layout(location = 0) in vec4 position;"
                                     "\n"
                                     "void main()\n"
                                     "{\n"
                                     "  gl_Position = vec4(position);\n"
                                     "}\n";

    const std::string fragmentShader = "#version 330 core\n"
                                       "\n"
                                       "layout(location = 0) out vec4 color;"
                                       "\n"
                                       "void main()\n"
                                       "{\n"
                                       "  color = vec4(1.0, 0.0, 0.0, 1.0);\n"
                                       "}\n";

    unsigned int shader = createShader(vertexShader, fragmentShader);
    glUseProgram(shader);

    while (_running) {

        glClear(GL_COLOR_BUFFER_BIT);

        glDrawArrays(GL_TRIANGLES, 0, 3);

        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED || event.type == SDL_EVENT_QUIT) {
                _running = false;
            }
        }

        SDL_GL_SwapWindow(_window);
    }
}