#pragma once

#include <SDL3/SDL.h>
#include <cstddef>
#include <cstdint>
#include <glad/glad.h>
#include <memory>

#include "ITexture.hpp"
#include "Model.hpp"
#include "Window.hpp"
#include "camera.hpp"
#include "math/Matrix4.hpp"
#include "renderer/IRenderer.hpp"

#include "IndexBuffer.hpp"
#include "ShaderOpenGL.hpp"
#include "TextureOpenGL.hpp"
#include "VertexBuffer.hpp"
#include <vector>

constexpr float CAMERA_SPEED = 10.0f;
constexpr float ROTATION_SPEED = 0.5f;
constexpr float BLEND_SPEED = 0.02f;
constexpr float FRAME_TIME = 1.0f / 30.0f;

// Ensure that if there is any GL error, it close the program and tell which GL
// error code happened. #x -> transform into a string. For development purpose.
#ifndef DEBUG

#define ASSERT(x)                                                                                                      \
    if (!(x))                                                                                                          \
        std::abort();
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
bool GlLogCall(const char* function, const char* file, uint32_t line);

class RendererOpenGL : public IRenderer
{

    public:
    RendererOpenGL() = delete;
    explicit RendererOpenGL(Window& window);
    ~RendererOpenGL();

    void Start() override;
    void Update(float deltaTime, Camera& camera) override;
    void Render() override;

    void SetPolygonMode(RenderMode mode) override;

    inline void SetRotationAxis(RotationAxis axis) override
    {
        _activeAxis = axis;
    }

    void SwapBuffers() override;

    inline void LoadModel(std::unique_ptr<Model> model) override
    {
        _model = std::move(model);
    }

    inline void LoadTexture(std::unique_ptr<ITexture> texture) override
    {
        _texture = std::move(texture);
    }

    inline void LoadNoiseTexture(std::unique_ptr<ITexture> texture) override
    {
        _noiseTexture = std::move(texture);
    }

    inline void PlayBadApple() override
    {
        _useBadAppleOnModel = !_useBadAppleOnModel;
    }

    inline void ApplyDissolve() override
    {
        _useDissolve = !_useDissolve;
    }

    inline void ApplyTexture() override
    {
        _transitioning = true;

        if (_blendFactor >= 1.0f)
        {
            _targetBlendFactor = 0.0f;
        }
        else
        {
            _targetBlendFactor = 1.0f;
        }
    }

    inline std::unique_ptr<ITexture> CreateTexture(const std::string& path) override
    {
        return std::make_unique<TextureOpenGL>(path);
    }

    virtual void SetFrame(size_t frame) override
    {
        if (frame >= _badAppleFrames.size())
        {
            frame = _badAppleFrames.size() - 1;
        }
        _currentFrame = frame;
        LoadFrameIfNeeded(_currentFrame);
    }

    Matrix4 GetFinalMatrix(RotationAxis activeAxis, const Matrix4& accumulatedRotationMatrix);

    private:
    SDL_GLContext _GLContext;

    Matrix4 _translateToOrigin;
    Matrix4 _translateBack;
    Matrix4 _accumulatedRotationMatrix = Matrix4(1.0f);
    Matrix4 _viewMatrix;
    Matrix4 _projectionMatrix;

    RotationAxis _activeAxis = RotationAxis::NONE;

    Window& _window;

    std::unique_ptr<VertexBuffer> _vb;
    std::unique_ptr<VertexBuffer> _quadVB;

    std::unique_ptr<IndexBuffer> _ib;
    std::unique_ptr<IndexBuffer> _quadIB;

    std::unique_ptr<ShaderOpenGL> _shader;
    std::unique_ptr<ShaderOpenGL> _quadShader;

    std::unique_ptr<ITexture> _texture;
    std::unique_ptr<ITexture> _noiseTexture;
    std::vector<std::unique_ptr<ITexture>> _badAppleFrames;

    std::unique_ptr<Model> _model;

    uint32_t _VAO = 0;
    uint32_t _quadVAO = 0;
    uint32_t _currentFrame = 0;

    float _frameTimer = 0.0f;
    float _dissolveAmount = 0.0f;
    float _blendFactor = 0.0f;
    float _targetBlendFactor = 0.0f;

    bool _useBadAppleOnModel = false;
    bool _useDissolve = false;
    bool _transitioning = false;

    void LoadFrameIfNeeded(std::size_t frameIndex);
};
