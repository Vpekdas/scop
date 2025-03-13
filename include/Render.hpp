#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_video.h>
#include <glad/glad.h>

#include <SDL3/SDL_opengl.h>

#include <iostream>

constexpr char W_TITLE[] = "SCOP";
constexpr int W_WIDTH = 1280;
constexpr int W_HEIGHT = 720;

class Render {
    public:
    Render();
    ~Render();
    void init();
    void mainLoop();
    unsigned int compileShader(unsigned int type, const std::string &source);
    int createShader(const std::string &vertexShader, const std::string &fragmentShader);

    private:
    SDL_Window *_window;
    SDL_GLContext _gl_context;
    bool _running;
    unsigned int _vao;
    unsigned int _vbo;

    protected:
};