#include "../include/Renderer.hpp"
#include "../include/colors.hpp"
#include "../include/vector.hpp"
#include "SDL3/SDL_error.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_init.h"
#include <alloca.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "../include/IndexBuffer.hpp"
#include "../include/Shader.hpp"
#include "../include/Texture.hpp"
#include "../include/VertexBuffer.hpp"

void GlClearError() {
    while (glGetError() != GL_NO_ERROR)
        ;
}

bool GlLogCall(const char *function, const char *file, unsigned int line) {
    while (GLenum error = glGetError()) {
        std::cerr << "[OpenGl Error] (" << error << "): " << function << " " << ":" << line << std::endl;
        return false;
    }
    return true;
}

Renderer::Renderer(const std::string &file) : _window(nullptr), _gl_context(nullptr), _running(true), _vao(0) {
    _model.parse(file);

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

    // Limiting frame rate.
    SDL_GL_SetSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        throw std::runtime_error(SDL_GetError());
    }

    std::cout << BOLD_ITALIC_CYAN << "Using Renderer: " << glGetString(GL_RENDERER) << " " << glGetString(GL_VERSION)
              << RESET << std::endl;
}

Renderer::~Renderer() {

    SDL_DestroyWindow(_window);
    SDL_GL_DestroyContext(_gl_context);
    SDL_Quit();
}

void Renderer::mainLoop() {

    GlCall(glEnable(GL_CULL_FACE));

    // Enable Depth Testing to ensure that depth is rendered correctly.
    // Ex: A face should not be visible on top of another face if it's behind.

    GlCall(glEnable(GL_DEPTH_TEST));
    GlCall(glDepthFunc(GL_LESS));

    GlCall(glEnable(GL_BLEND));
    GlCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    // Create and bind Vertex Array.
    GlCall(glGenVertexArrays(1, &_vao));
    GlCall(glBindVertexArray(_vao));

    // Create and bind Vertex Buffer.
    VertexBuffer vb(_model._combinedVertexBuffer.data(), _model._combinedVertexBuffer.size() * sizeof(float));

    // Set up vertex attribute pointers.
    // Vertex positions (attribute 0)
    GlCall(glEnableVertexAttribArray(0));
    GlCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (const void *)(0)));

    // Texture coordinates (attribute 1)
    GlCall(glEnableVertexAttribArray(1));
    GlCall(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (const void *)(3 * sizeof(float))));

    // Create and bind Vertex Indices Buffer.
    IndexBuffer ib(_model._vertexIndices.data(), _model._vertexIndices.size());

    float angle = 0.0f;

    // Set Initial Camera position.
    Matrix4 viewMatrix = Matrix4::translation(Vector3(0.0f, 0.0f, -5.0f));
    Matrix4 projectionMatrix = Matrix4::perspective(45.0f, W_WIDTH, W_HEIGHT, 0.1f, 100.0f);

    _model.calculateCentroid();

    // Get the centroid of the model.
    Vector3 centroid = _model._centroid;

    Shader shader("../res/Basic.glsl");
    shader.Bind();
    shader.setUniformMat4f("u_ViewMatrix", viewMatrix);
    shader.setUniformMat4f("u_ProjectionMatrix", projectionMatrix);

    Texture texture("../models/test.tga");

    texture.Bind();

    // 0 because slot 0.
    shader.setUniform1i("u_Texture", 0);

    GlCall(glBindVertexArray(0));
    vb.Unbind();
    ib.Unbind();
    shader.Unbind();

    while (_running) {
        angle += 0.5f;

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED || event.type == SDL_EVENT_QUIT) {
                _running = false;
            } else if (event.type == SDL_EVENT_KEY_DOWN) {
            }
        }

        GlCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

        // Translate to origin 0, 0, 0.
        Matrix4 translateToOrigin = Matrix4::translation(-centroid);

        // Apply rotation.
        Matrix4 rotationMatrix = Matrix4::rotationY(angle);

        // Translate back to original position.
        Matrix4 translateBack = Matrix4::translation(centroid);

        Matrix4 modelMatrix = translateToOrigin * rotationMatrix * translateBack;

        shader.Bind();
        shader.setUniformMat4f("u_ModelMatrix", modelMatrix);

        GlCall(glBindVertexArray(_vao));
        ib.Bind();

        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        GlCall(glDrawElements(GL_TRIANGLES, _model._vertexIndices.size(), GL_UNSIGNED_INT, nullptr));

        SDL_GL_SwapWindow(_window);
    }
}