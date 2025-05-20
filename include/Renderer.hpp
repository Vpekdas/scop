#pragma once

#include <SDL3/SDL.h>
#include <glad/glad.h>

#include "Matrix4.hpp"
#include "Model.hpp"
#include "vector.hpp"
#include <signal.h>

constexpr char W_TITLE[] = "SCOP";
constexpr int W_WIDTH = 1280;
constexpr int W_HEIGHT = 720;
constexpr float CAMERA_SPEED = 10.0f;
constexpr float ROTATION_SPEED = 0.5f;
constexpr float BLEND_SPEED = 0.02f;

// Ensure that if there is any GL error, it close the program and tell which GL
// error code happened. #x -> transform into a string. For development purpose.
#ifndef DEBUG

#define ASSERT(x)                                                                                                      \
    if (!(x)) std::abort();
#define GlCall(x)                                                                                                      \
    GlClearError();                                                                                                    \
    x;                                                                                                                 \
    ASSERT(GlLogCall(#x, __FILE__, __LINE__))

#else
#define ASSERT(x) ((void)0)
#define GlCall(x) x
#endif

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
    void mainLoop();

    void handleInput(const SDL_Event &event, bool &run, bool &trans, RotationAxis &axis, float &blendFactor);

    void handleTextureTrans(bool &transitioning, float &blendFactor, float &targetBlendFactor);

    void handleBackgroundColor(float &red, float &green);

    // Multiplies the accumulated rotation matrix with a new rotation matrix along
    // the specified axis (X, Y, or Z). This allows continuous rotation of a model
    // around a single axis using the existing coordinate system.
    Matrix4 accumulatedMatrix(RotationAxis activeAxis, const Matrix4 &accumulatedRotationMatrix);

    private:
    SDL_Window *_window;
    SDL_GLContext _GLContext;
    unsigned int _VAO;
    Model _model;
    std::string _texturePath;
};
