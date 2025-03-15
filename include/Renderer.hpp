#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_video.h>
#include <glad/glad.h>

#include <SDL3/SDL_opengl.h>

#include "../include/Matrix4.hpp"
#include "../include/Model.hpp"
#include "../include/VertexBuffer.hpp"
#include "IndexBuffer.hpp"
#include "Shader.hpp"
#include <iostream>
#include <signal.h>

constexpr char W_TITLE[] = "SCOP";
constexpr int W_WIDTH = 1280;
constexpr int W_HEIGHT = 720;

// Ensure that if there is any GL error, it close the program and tell which GL error code happened.
// #x -> transform into a string.
#define ASSERT(x)                                                                                                      \
    if (!(x))                                                                                                          \
        raise(SIGTRAP);
#define GlCall(x)                                                                                                      \
    GlClearError();                                                                                                    \
    x;                                                                                                                 \
    ASSERT(GlLogCall(#x, __FILE__, __LINE__))

void GlClearError();
bool GlLogCall(const char *function, const char *file, unsigned int line);

class Renderer {
    public:
    Renderer(const std::string &filename);
    ~Renderer();
    void mainLoop();

    private:
    SDL_Window *_window;
    SDL_GLContext _gl_context;
    bool _running;
    unsigned int _vao;
    Model _model;
};
