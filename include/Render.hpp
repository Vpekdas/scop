#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_video.h>
#include <glad/glad.h>

#include <SDL3/SDL_opengl.h>

#include "../include/Model.hpp"
#include "Matrix4.hpp"
#include <iostream>

constexpr char W_TITLE[] = "SCOP";
constexpr int W_WIDTH = 1280;
constexpr int W_HEIGHT = 720;

class Render {
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

    Render(const std::string &filename);
    ~Render();
    void init();
    void mainLoop();
    unsigned int compileShader(unsigned int type, const std::string &source);
    int createShader(const std::string &vertexShader, const std::string &fragmentShader);
    ShaderProgramSource parseShader(const std::string &file);

    private:
    SDL_Window *_window;
    SDL_GLContext _gl_context;
    bool _running;
    unsigned int _vao;
    unsigned int _vbo;
    unsigned int _ebo;
    Model _model;

    GLint _modelMatrixLocation;
    GLint _projectionMatrixLocation;

    Matrix4 _modelMatrix;
    Matrix4 _projectionMatrix;

    protected:
};
