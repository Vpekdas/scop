#include "../include/Renderer.hpp"
#include "../include/IndexBuffer.hpp"
#include "../include/Shader.hpp"
#include "../include/Texture.hpp"
#include "../include/VertexBuffer.hpp"
#include "../include/camera.hpp"
#include "../include/vector.hpp"

#include <alloca.h>
#include <filesystem>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

static Uint64 lastTime = SDL_GetPerformanceCounter();

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
    : _window(nullptr), _GLContext(nullptr), _VAO(0), _model(), _texturePath(texturePath) {

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

    _GLContext = SDL_GL_CreateContext(_window);

    if (!_GLContext) {
        throw std::runtime_error(SDL_GetError());
    }

    SDL_GL_MakeCurrent(_window, _GLContext);

    // Limiting frame rate (VSync).
    SDL_GL_SetSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        throw std::runtime_error(SDL_GetError());
    }

    std::cout << "Using Renderer: " << glGetString(GL_RENDERER) << " " << glGetString(GL_VERSION) << std::endl;
}

Renderer::~Renderer() {
    SDL_DestroyWindow(_window);
    SDL_GL_DestroyContext(_GLContext);
    SDL_Quit();
}

// [&] captures all variables by reference, allowing them to be modified.
void Renderer::handleInput(const SDL_Event &event, bool &run, bool &trans, RotationAxis &axis, float &blendFactor) {

    // Creates a map of function.
    static const std::unordered_map<int, std::function<void()>> keyBindings = {
        {SDLK_F1, []() { glPolygonMode(GL_FRONT_AND_BACK, GL_POINT); }},
        {SDLK_F2, []() { glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); }},
        {SDLK_F3, []() { glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); }},
        {SDLK_ESCAPE, [&]() { run = false; }},
        {SDLK_T,
         [&trans, &blendFactor]() {
             trans = true;
             blendFactor = (blendFactor == 1.0f) ? 0.0f : 1.0f;
         }},
        {SDLK_X, [&]() { axis = RotationAxis::X; }},
        {SDLK_Y, [&]() { axis = RotationAxis::Y; }},
        {SDLK_Z, [&]() { axis = RotationAxis::Z; }},
        {SDLK_SPACE, [&]() { axis = RotationAxis::NONE; }},
    };

    auto it = keyBindings.find(event.key.key);
    if (it != keyBindings.end()) {
        it->second();
    }
}

Matrix4 Renderer::accumulatedMatrix(RotationAxis activeAxis, const Matrix4 &accumulatedRotationMatrix) {
    Matrix4 rotationMatrix = Matrix4(1.0f);

    switch (activeAxis) {
    case RotationAxis::X:
        rotationMatrix = Matrix4::rotationX(ROTATION_SPEED);
        break;
    case RotationAxis::Y:
        rotationMatrix = Matrix4::rotationY(ROTATION_SPEED);
        break;
    case RotationAxis::Z:
        rotationMatrix = Matrix4::rotationZ(ROTATION_SPEED);
        break;
    case RotationAxis::NONE:
        break;
    }

    return accumulatedRotationMatrix * rotationMatrix;
}

void Renderer::handleTextureTrans(bool &transitioning, float &blendFactor, float &targetBlendFactor) {
    if (transitioning) {
        if (blendFactor < targetBlendFactor) {
            blendFactor += BLEND_SPEED;
            if (blendFactor >= targetBlendFactor) {
                blendFactor = targetBlendFactor;
                transitioning = false;
            }
        } else if (blendFactor > targetBlendFactor) {
            blendFactor -= BLEND_SPEED;
            if (blendFactor <= targetBlendFactor) {
                blendFactor = targetBlendFactor;
                transitioning = false;
            }
        }
    }
}

void Renderer::handleBackgroundColor(float &red, float &green) {
    red = (sin(SDL_GetTicks() * 0.001f) + 1.0f) / 2.0f;
    green = (cos(SDL_GetTicks() * 0.001f) + 1.0f) / 2.0f;
    GlCall(glClearColor(red, green, 0.5f, 1.0f));
    GlCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
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

    GlCall(glGenVertexArrays(1, &_VAO));
    GlCall(glBindVertexArray(_VAO));

    VertexBuffer vb(_model._vertexBuffer.data(), _model._vertexBuffer.size() * sizeof(float));

    // Set up vertex attribute pointers. The first 3 float are the positions.
    // Vertex positions (attribute 0) on shader.
    GlCall(glEnableVertexAttribArray(0));
    GlCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (const void *)(0)));

    // The next 2 float are the texture coordinates.
    // Texture coordinates (attribute 1) on shader.
    GlCall(glEnableVertexAttribArray(1));
    GlCall(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (const void *)(3 * sizeof(float))));

    IndexBuffer ib(_model._verticesIndices.data(), _model._verticesIndices.size());

    Camera camera(0.0f, 0.0f, 10.0f);
    Matrix4 viewMatrix = Matrix4::translation(-camera.pos);
    Matrix4 projectionMatrix = Matrix4::perspective(45.0f, W_WIDTH, W_HEIGHT, 0.1f, 1000.0f);

    Vector3 centroid = _model._centroid;

    std::filesystem::path shaderPath = std::filesystem::current_path() / "res" / "Basic.glsl";

    Shader shader(shaderPath);
    shader.Bind();
    shader.setUniformMat4f("u_ViewMatrix", viewMatrix);
    shader.setUniformMat4f("u_ProjectionMatrix", projectionMatrix);

    Texture texture(_texturePath);
    texture.Bind();
    shader.setUniform1i("u_Texture", 0);

    GlCall(glBindVertexArray(0));
    vb.Unbind();
    ib.Unbind();
    shader.Unbind();

    float red = 0.0f, green = 0.0f;
    RotationAxis activeAxis = RotationAxis::NONE;

    Matrix4 translateToOrigin = Matrix4::translation(-centroid);
    Matrix4 translateBack = Matrix4::translation(centroid);
    Matrix4 accumulatedRotationMatrix = Matrix4(1.0f);

    float deltaTime = 0.0f;
    float blendFactor = 0.0f, targetBlendFactor = 0.0f;
    bool running = true, transitioning = false;

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED || event.type == SDL_EVENT_QUIT) {
                running = false;
            } else if (event.type == SDL_EVENT_KEY_DOWN) {
                handleInput(event, running, transitioning, activeAxis, targetBlendFactor);
            }
        }

        Uint64 currentTime = SDL_GetPerformanceCounter();
        deltaTime = (currentTime - lastTime) / (float)SDL_GetPerformanceFrequency();
        lastTime = currentTime;

        camera.handleInput(deltaTime);

        handleTextureTrans(transitioning, blendFactor, targetBlendFactor);

        handleBackgroundColor(red, green);

        shader.Bind();
        shader.setUniform1f("u_ModeFactor", blendFactor);

        // Rotation for camera with arrow key.
        viewMatrix = Matrix4::rotationY(camera.rotationAngle) * Matrix4::translation(-camera.pos);
        shader.setUniformMat4f("u_ViewMatrix", viewMatrix);

        accumulatedRotationMatrix = accumulatedMatrix(activeAxis, accumulatedRotationMatrix);
        Matrix4 modelMatrix = translateToOrigin * accumulatedRotationMatrix * translateBack;
        shader.setUniformMat4f("u_ModelMatrix", modelMatrix);

        GlCall(glBindVertexArray(_VAO));
        ib.Bind();

        GlCall(glDrawElements(GL_TRIANGLES, _model._verticesIndices.size(), GL_UNSIGNED_INT, nullptr));

        SDL_GL_SwapWindow(_window);
    }
}