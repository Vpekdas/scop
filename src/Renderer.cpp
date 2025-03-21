#include "../include/Renderer.hpp"
#include "../include/IndexBuffer.hpp"
#include "../include/Shader.hpp"
#include "../include/Texture.hpp"
#include "../include/VertexBuffer.hpp"
#include "../include/colors.hpp"
#include "../include/vector.hpp"
#include "SDL3/SDL_keycode.h"

#include <alloca.h>
#include <string>
#include <vector>

void GlClearError() {
    while (glGetError() != GL_NO_ERROR)
        ;
}

bool GlLogCall(const char *function, const char *file, unsigned int line) {
    while (GLenum error = glGetError()) {
        std::cerr << "[OpenGl Error] (" << error << "): " << function << " " << ":" << file << " " << line << std::endl;
        return false;
    }
    return true;
}

Renderer::Renderer(const std::string &file, const std::string &texturePath)
    : _window(nullptr), _gl_context(nullptr), _vao(0), _model(), _texturePath(texturePath), _running(true),
      _textureMode(false) {

    _model.parse(file);

    if (!SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO)) {
        throw std::runtime_error(SDL_GetError());
    }

    // https://stackoverflow.com/questions/19865463/opengl-4-1-under-mavericks
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    // Create Window.
    _window = SDL_CreateWindow(W_TITLE, W_WIDTH, W_HEIGHT, SDL_WINDOW_OPENGL);

    if (!_window) {
        throw std::runtime_error(SDL_GetError());
    }

    // Create an OpenGL rendering context for the window.
    _gl_context = SDL_GL_CreateContext(_window);

    if (!_gl_context) {
        throw std::runtime_error(SDL_GetError());
    }

    SDL_GL_MakeCurrent(_window, _gl_context);

    // Limiting frame rate (VSync).
    SDL_GL_SetSwapInterval(1);

    // Initialize Glad so we can use OpenGL functions.
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

    // Ensure that back-facing faces are not rendered to improve performance.
    GlCall(glEnable(GL_CULL_FACE));

    // Enable Depth Testing to ensure that depth is rendered correctly.
    // Ex: A face should not be visible on top of another face if it's behind.
    GlCall(glEnable(GL_DEPTH_TEST));
    GlCall(glDepthFunc(GL_LESS));

    // Enable transparency.
    GlCall(glEnable(GL_BLEND));
    GlCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    // Create and bind Vertex Array.
    // Configure the Vertex Array Object to tell OpenGL how to interpret the vertex buffer data.
    GlCall(glGenVertexArrays(1, &_vao));
    GlCall(glBindVertexArray(_vao));

    // Create and bind Vertex Buffer.
    // It contains 3 float x, y and z as positions then u and v as texture coordinates.
    VertexBuffer vb(_model._combinedVertexBuffer.data(), _model._combinedVertexBuffer.size() * sizeof(float));

    // Set up vertex attribute pointers. The first 3 float are the positions.
    // Vertex positions (attribute 0) on shader.
    GlCall(glEnableVertexAttribArray(0));
    GlCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (const void *)(0)));

    // The next 2 float are the texture coordinates.
    // Texture coordinates (attribute 1) on shader.
    GlCall(glEnableVertexAttribArray(1));
    GlCall(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (const void *)(3 * sizeof(float))));

    // Create and bind Vertex Indices Buffer.
    // OpenGL uses the index buffer to reference vertices and draw triangles efficiently.
    IndexBuffer ib(_model._vertexIndices.data(), _model._vertexIndices.size());

    float angle = 0.0f;

    // Set Initial Camera position.
    Vector3 camera(0, 0, 5);
    Matrix4 viewMatrix = Matrix4::translation(-camera);
    Matrix4 projectionMatrix = Matrix4::perspective(45.0f, W_WIDTH, W_HEIGHT, 0.1f, 1000.0f);

    // Usefull for rotating model by his mass center.
    _model.calculateCentroid();

    // Get the centroid of the model.
    Vector3 centroid = _model._centroid;

    // Set the shader that display faces.
    Shader _faceShader("./res/Face.glsl");
    _faceShader.Bind();
    _faceShader.setUniformMat4f("u_ViewMatrix", viewMatrix);
    _faceShader.setUniformMat4f("u_ProjectionMatrix", projectionMatrix);
    _faceShader.Unbind();

    // Set the shader that display a texture.
    Shader _texturedShader("./res/Textured.glsl");
    _texturedShader.Bind();
    _texturedShader.setUniformMat4f("u_ViewMatrix", viewMatrix);
    _texturedShader.setUniformMat4f("u_ProjectionMatrix", projectionMatrix);
    Texture texture(_texturePath);
    texture.Bind();
    _texturedShader.setUniform1i("u_Texture", 0);

    // Unbind all objects.
    GlCall(glBindVertexArray(0));
    vb.Unbind();
    ib.Unbind();
    _faceShader.Unbind();
    _texturedShader.Unbind();

    while (_running) {

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED || event.type == SDL_EVENT_QUIT) {
                _running = false;
            } else if (event.type == SDL_EVENT_KEY_DOWN) {
                // TODO: Refactor this.
                if (event.key.key == SDLK_T) {
                    _textureMode = true;
                }
                if (event.key.key == SDLK_F) {
                    _textureMode = false;
                }
                if (event.key.key == SDLK_P) {
                    glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
                }
                if (event.key.key == SDLK_L) {
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                }
                if (event.key.key == SDLK_C) {
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                }
                if (event.key.key == SDLK_ESCAPE) {
                    _running = false;
                }
                if (event.key.key == SDLK_S) {
                    camera.z += VELOCITY;
                }
                if (event.key.key == SDLK_W) {
                    camera.z -= VELOCITY;
                }
                if (event.key.key == SDLK_D) {
                    camera.x += VELOCITY;
                }
                if (event.key.key == SDLK_A) {
                    camera.x -= VELOCITY;
                }
                if (event.key.key == SDLK_UP) {
                    camera.y += VELOCITY;
                }
                if (event.key.key == SDLK_DOWN) {
                    camera.y -= VELOCITY;
                }
            }
        }

        Matrix4 viewMatrix = Matrix4::translation(-camera);

        // Translate to origin 0, 0, 0.
        Matrix4 translateToOrigin = Matrix4::translation(-centroid);

        // Apply rotation.
        Matrix4 rotationMatrix = Matrix4::rotationY(angle);

        // Translate back to original position.
        Matrix4 translateBack = Matrix4::translation(centroid);

        Matrix4 modelMatrix = translateToOrigin * rotationMatrix * translateBack;

        angle += 0.5f;

        GlCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

        // Select either the shader that display face or texture.
        if (_textureMode) {
            _texturedShader.Bind();
            _texturedShader.setUniformMat4f("u_ViewMatrix", viewMatrix);
            _texturedShader.setUniformMat4f("u_ModelMatrix", modelMatrix);
        }

        else {
            _faceShader.Bind();
            _faceShader.setUniformMat4f("u_ViewMatrix", viewMatrix);
            _faceShader.setUniformMat4f("u_ModelMatrix", modelMatrix);
        }

        // Select which buffer will be used to render.
        GlCall(glBindVertexArray(_vao));
        ib.Bind();

        // Draw triangle by looking at indices.
        GlCall(glDrawElements(GL_TRIANGLES, _model._vertexIndices.size(), GL_UNSIGNED_INT, nullptr));

        SDL_GL_SwapWindow(_window);
    }
}