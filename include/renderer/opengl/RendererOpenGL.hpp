#pragma once

#include <SDL3/SDL.h>
#include <cstddef>
#include <cstdint>
#include <glad/glad.h>
#include <memory>

#include "ITexture.hpp"
#include "Mesh.hpp"
#include "Window.hpp"
#include "camera.hpp"
#include "math/Matrix4.hpp"
#include "renderer/IRenderer.hpp"

#include "IndexBuffer.hpp"
#include "ShaderOpenGL.hpp"
#include "TextureOpenGL.hpp"
#include "VertexBuffer.hpp"

constexpr float CAMERA_SPEED = 10.0f;
constexpr float ROTATION_SPEED = 0.5f;
constexpr float BLEND_SPEED = 0.02f;

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

    inline void LoadMesh(std::unique_ptr<Mesh> mesh) override
    {
        _mesh = std::move(mesh);
    }

    inline void LoadTexture(std::unique_ptr<ITexture> texture) override
    {
        _texture = std::move(texture);
    }

    inline void LoadNoiseTexture(std::unique_ptr<ITexture> texture) override
    {
        _noiseTexture = std::move(texture);
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

    std::unique_ptr<IndexBuffer> _ib;

    std::unique_ptr<ShaderOpenGL> _textureShader;
    std::unique_ptr<ShaderOpenGL> _dissolveShader;

    std::unique_ptr<ITexture> _texture;
    std::unique_ptr<ITexture> _noiseTexture;

    std::unique_ptr<Mesh> _mesh;

    uint32_t _VAO = 0;

    float _dissolveAmount = 0.0f;
    float _blendFactor = 0.0f;
    float _targetBlendFactor = 0.0f;

    bool _useDissolve = false;
    bool _transitioning = false;
};
