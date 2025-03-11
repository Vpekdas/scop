#include "../include/Render.hpp"
#include "../include/colors.hpp"
#include "SDL3/SDL_error.h"
#include "SDL3/SDL_init.h"
#include <iostream>

Render::Render() : _window(nullptr), _gl_context(nullptr) {
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