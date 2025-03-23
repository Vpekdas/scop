#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_video.h>
#include <glad/glad.h>

#include <SDL3/SDL_opengl.h>

#include "Model.hpp"
#include "vector.hpp"
#include <signal.h>

constexpr char W_TITLE[] = "SCOP";
constexpr int W_WIDTH = 1280;
constexpr int W_HEIGHT = 720;
constexpr float CAMERA_SPEED = 0.7f;
constexpr float ROTATION_SPEED = 0.5f;

// Ensure that if there is any GL error, it close the program and tell which GL error code happened.
// #x -> transform into a string.
// For development purpose.
#define ASSERT(x)                                                                                                      \
    if (!(x))                                                                                                          \
        raise(SIGTRAP);
#define GlCall(x)                                                                                                      \
    GlClearError();                                                                                                    \
    x;                                                                                                                 \
    ASSERT(GlLogCall(#x, __FILE__, __LINE__))

// Extract OpenGL errors contained in queue.
void GlClearError();
// DIsplay OpenGl error code, function name and line.
bool GlLogCall(const char *function, const char *file, unsigned int line);

class Renderer {
    public:
    enum class RotationAxis {
        NONE,
        X,
        Y,
        Z
    };

    Renderer(const std::string &filename, const std::string &texturePath);
    ~Renderer();

    // Set up OpenGL state (culling, depth testing, transparency).
    // Initialize Vertex Array, Vertex Buffer, Index Buffer, and Shader.
    // Handle input events, update matrices, and run the render loop.
    void mainLoop();

    void handleInput(const SDL_Event &event, bool &running, bool &textureMode, Vector3 &camera,
                     RotationAxis &activeAxis, float &cameraRotationAngle);

    private:
    SDL_Window *_window;
    SDL_GLContext _gl_context;
    unsigned int _vao;
    Model _model;
    std::string _texturePath;
};
