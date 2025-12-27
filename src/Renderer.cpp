#include "../include/Renderer.hpp"
#include "../include/IndexBuffer.hpp"
#include "../include/Shader.hpp"
#include "../include/Texture.hpp"
#include "../include/VertexBuffer.hpp"
#include "../include/camera.hpp"
#include "../include/vector.hpp"
#include "SDL3/SDL_init.h"

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

Renderer::Renderer() : _window(nullptr), _GLContext(nullptr), _camera(0.0f, 0.0f, 10.0f) {

    _badAppleFrames.resize(6572);

    if (!SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
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

    std::filesystem::path wawPath = std::filesystem::current_path() / "assets" / "sounds" / "bad_apple.wav";

    if (!SDL_LoadWAV(wawPath.c_str(), &_spec, &_wavData, &_wavDataLen)) {
        SDL_Log("Couldn't load .wav file: %s", SDL_GetError());
    }

    /* Create our audio stream in the same format as the .wav file. It'll convert to what the audio hardware wants. */
    _stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &_spec, NULL, NULL);
    if (!_stream) {
        SDL_Log("Couldn't create audio stream: %s", SDL_GetError());
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

void Renderer::Input(const SDL_Event &event, bool &run, bool &trans, RotationAxis &axis, float &blendFactor) {

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
        {SDLK_B, [&]() { _useBadAppleOnModel = !_useBadAppleOnModel; }},
        {SDLK_P, [&]() { _useDissolve = !_useDissolve; }},

    };

    auto it = keyBindings.find(event.key.key);
    if (it != keyBindings.end()) {
        it->second();
    }
}

Matrix4 Renderer::GetFinalMatrix(RotationAxis activeAxis, const Matrix4 &accumulatedRotationMatrix) {
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

void Renderer::LoadFrameIfNeeded(std::size_t frameIndex) {
    if (!_badAppleFrames[frameIndex]) {
        std::filesystem::path path =
            std::filesystem::current_path() / "assets" / "bad_apple" / (std::to_string(frameIndex + 1) + ".tga");

        _badAppleFrames[frameIndex] = std::make_unique<Texture>(path);
    }
}

void Renderer::Loop() {
    Start();

    Uint64 lastTime = SDL_GetPerformanceCounter();
    float accumulatedTime = 0.0f;
    const float frameDuration = 1.0f / 30.0f;

    while (_running) {
        Uint64 currentTime = SDL_GetPerformanceCounter();
        float deltaTime = (currentTime - lastTime) / (float)SDL_GetPerformanceFrequency();
        lastTime = currentTime;

        accumulatedTime += deltaTime;

        if (accumulatedTime >= frameDuration) {
            _currentFrame = (_currentFrame + 1) % _badAppleFrames.size();
            accumulatedTime -= frameDuration;
            LoadFrameIfNeeded(_currentFrame);
        }

        Update(deltaTime);
        Render();
    }
}

void Renderer::Start() {
    GlCall(glEnable(GL_CULL_FACE));
    GlCall(glEnable(GL_DEPTH_TEST));
    GlCall(glDepthFunc(GL_LESS));
    GlCall(glEnable(GL_BLEND));
    GlCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    GlCall(glGenVertexArrays(1, &_VAO));
    GlCall(glBindVertexArray(_VAO));

    _vb = std::make_unique<VertexBuffer>(_model->_vertexBuffer.data(), _model->_vertexBuffer.size() * sizeof(float));
    _ib = std::make_unique<IndexBuffer>(_model->_verticesIndices.data(), _model->_verticesIndices.size());

    // Set up vertex attribute pointers. The first 3 float are the positions.
    // Vertex positions (attribute 0) on shader.
    GlCall(glEnableVertexAttribArray(0));
    GlCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (const void *)(0)));

    // The next 2 float are the texture coordinates.
    // Texture coordinates (attribute 1) on shader.
    GlCall(glEnableVertexAttribArray(1));
    GlCall(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (const void *)(3 * sizeof(float))));

    _texture->Bind();
    _noiseTexture->Bind(1);

    std::filesystem::path shaderPath = std::filesystem::current_path() / "res" / "Basic.glsl";
    _viewMatrix = Matrix4::translation(-_camera.pos);
    _projectionMatrix = Matrix4::perspective(45.0f, W_WIDTH, W_HEIGHT, 0.1f, 1000.0f);
    _translateToOrigin = Matrix4::translation(-_model->_centroid);
    _translateBack = Matrix4::translation(_model->_centroid);

    _shader = std::make_unique<Shader>(shaderPath);
    _shader->Bind();
    _shader->setUniform1i("u_Texture", 0);
    _shader->setUniform1i("u_DissolveTexture", 0);
    _shader->setUniformMat4f("u_ViewMatrix", _viewMatrix);
    _shader->setUniformMat4f("u_ProjectionMatrix", _projectionMatrix);

    std::filesystem::path quadShaderPath = std::filesystem::current_path() / "res" / "Quad.glsl";
    _quadShader = std::make_unique<Shader>(quadShaderPath);
    _quadShader->Bind();
    _quadShader->setUniform1i("u_Texture", 0);

    const float quadVertices[] = {-1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,  -1.0f, 0.0f, 1.0f, 1.0f,
                                  1.0f,  1.0f,  0.0f, 1.0f, 0.0f, -1.0f, 1.0f,  0.0f, 0.0f, 0.0f};

    const unsigned int quadIndices[] = {0, 1, 2, 2, 3, 0};

    GlCall(glGenVertexArrays(1, &_quadVAO));
    GlCall(glBindVertexArray(_quadVAO));

    _quadVB = std::make_unique<VertexBuffer>(quadVertices, sizeof(quadVertices));
    _quadIB = std::make_unique<IndexBuffer>(quadIndices, 6);

    GlCall(glEnableVertexAttribArray(0));
    GlCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0));

    GlCall(glEnableVertexAttribArray(1));
    GlCall(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float))));

    LoadFrameIfNeeded(_currentFrame);

    SDL_PutAudioStreamData(_stream, _wavData, _wavDataLen);
    SDL_ResumeAudioStreamDevice(_stream);
}

void Renderer::Update(float deltaTime) {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED || event.type == SDL_EVENT_QUIT) _running = false;
        else if (event.type == SDL_EVENT_KEY_DOWN)
            Input(event, _running, _transitioning, _activeAxis, _targetBlendFactor);
        else if (_stream && SDL_GetAudioStreamQueued(_stream) == 0) {
            SDL_PutAudioStreamData(_stream, _wavData, _wavDataLen);
        }
    }

    _camera.Move(deltaTime);

    if (_transitioning) {
        if (_blendFactor < _targetBlendFactor) {
            _blendFactor += BLEND_SPEED;
            if (_blendFactor >= _targetBlendFactor) {
                _blendFactor = _targetBlendFactor;
                _transitioning = false;
            }
        } else if (_blendFactor > _targetBlendFactor) {
            _blendFactor -= BLEND_SPEED;
            if (_blendFactor <= _targetBlendFactor) {
                _blendFactor = _targetBlendFactor;
                _transitioning = false;
            }
        }
    }

    if (_useDissolve) {
        _dissolveAmount += deltaTime * 0.75f;
        if (_dissolveAmount > 1.0f) _dissolveAmount = 1.0f;
    } else {
        _dissolveAmount -= deltaTime * 0.75f;
        if (_dissolveAmount < 0.0f) _dissolveAmount = 0.0f;
    }

    _viewMatrix = Matrix4::rotationY(_camera.rotationAngle) * Matrix4::translation(-_camera.pos);
    _accumulatedRotationMatrix = GetFinalMatrix(_activeAxis, _accumulatedRotationMatrix);
}

void Renderer::Render() {

    if (_useBadAppleOnModel) {
        glClearColor(0.376f, 0.647f, 0.980f, 1.0f);
    }

    GlCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    Matrix4 modelMatrix = _translateToOrigin * _accumulatedRotationMatrix * _translateBack;

    if (!_useBadAppleOnModel) {
        GlCall(glDisable(GL_DEPTH_TEST));
        GlCall(glBindVertexArray(_quadVAO));
        _quadShader->Bind();
        _badAppleFrames[_currentFrame]->Bind();
        GlCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));
        GlCall(glEnable(GL_DEPTH_TEST));
    }

    _shader->Bind();

    if (_useBadAppleOnModel) {
        _badAppleFrames[_currentFrame]->Bind();
    } else {
        _texture->Bind();
    }

    _shader->setUniform1f("u_ModeFactor", _blendFactor);
    _shader->setUniform1f("u_DissolveAmount", _dissolveAmount);
    _shader->setUniformMat4f("u_ViewMatrix", _viewMatrix);
    _shader->setUniformMat4f("u_ModelMatrix", modelMatrix);

    GlCall(glBindVertexArray(_VAO));
    _ib->Bind();
    GlCall(glDrawElements(GL_TRIANGLES, _model->_verticesIndices.size(), GL_UNSIGNED_INT, nullptr));

    SDL_GL_SwapWindow(_window);
}
