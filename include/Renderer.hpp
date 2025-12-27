#pragma once

#include <SDL3/SDL.h>
#include <cstddef>
#include <glad/glad.h>
#include <memory>
#include <vector>

#include "IndexBuffer.hpp"
#include "Matrix4.hpp"
#include "Model.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "VertexBuffer.hpp"
#include "camera.hpp"

constexpr char W_TITLE[] = "SCOP";
constexpr int W_WIDTH = 1280;
constexpr int W_HEIGHT = 720;

constexpr float CAMERA_SPEED = 10.0f;
constexpr float ROTATION_SPEED = 0.5f;
constexpr float BLEND_SPEED = 0.02f;

constexpr int MAX_CACHED_FRAMES = 50;

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
// Display OpenGl error code, function name and line.
bool GlLogCall(const char *function, const char *file, unsigned int line);

class Renderer {
    public:
    enum class RotationAxis {
        NONE,
        X,
        Y,
        Z
    };

    Renderer();
    ~Renderer();

    void Loop();
    void Start();
    void Update(float deltaTime);
    void Render();
    void Input(const SDL_Event &event, bool &run, bool &trans, RotationAxis &axis, float &blendFactor);
    void OnTextureTransition(bool &transitioning, float &blendFactor, float &targetBlendFactor);

    void SetModel(std::unique_ptr<Model> model) {
        _model = std::move(model);
    }

    void SetTexture(std::unique_ptr<Texture> texture) {
        _texture = std::move(texture);
    }

    void SetNoiseTexture(std::unique_ptr<Texture> texture) {
        _noiseTexture = std::move(texture);
    }

    Matrix4 GetFinalMatrix(RotationAxis activeAxis, const Matrix4 &accumulatedRotationMatrix);

    private:
    SDL_Window *_window;
    SDL_GLContext _GLContext;
    Camera _camera;

    bool _running = true;
    bool _transitioning = false;
    RotationAxis _activeAxis = RotationAxis::NONE;

    float _blendFactor = 0.0f;
    float _targetBlendFactor = 0.0f;

    Matrix4 _translateToOrigin;
    Matrix4 _translateBack;
    Matrix4 _accumulatedRotationMatrix = Matrix4(1.0f);
    Matrix4 _viewMatrix;
    Matrix4 _projectionMatrix;

    std::unique_ptr<VertexBuffer> _vb;
    std::unique_ptr<VertexBuffer> _quadVB;

    std::unique_ptr<IndexBuffer> _ib;
    std::unique_ptr<IndexBuffer> _quadIB;

    std::unique_ptr<Shader> _shader;
    std::unique_ptr<Shader> _quadShader;

    std::unique_ptr<Texture> _texture;
    std::unique_ptr<Texture> _noiseTexture;
    std::unique_ptr<Model> _model;

    std::vector<std::unique_ptr<Texture>> _badAppleFrames;

    unsigned int _VAO = 0;
    unsigned int _quadVAO = 0;
    std::size_t _currentFrame = 0;

    float _frameTimer = 0.0f;
    const float _frameDuration = 1.0f / 30.0f;

    SDL_AudioSpec _spec;
    Uint8 *_wavData = NULL;
    Uint32 _wavDataLen = 0;
    SDL_AudioStream *_stream = NULL;

    bool _useBadAppleOnModel = false;
    bool _useDissolve = false;
    float _dissolveAmount = 0.0f;

    void LoadFrameIfNeeded(std::size_t frameIndex);
};
