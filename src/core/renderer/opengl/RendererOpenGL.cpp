#include <cstddef>
#include "renderer/opengl/RendererOpenGL.hpp"
#include "renderer/opengl/IndexBuffer.hpp"
#include "renderer/opengl/ShaderOpenGL.hpp"
#include "renderer/opengl/TextureOpenGL.hpp"
#include <cstdio>
#include <print>

#include "camera.hpp"
#include "math/vector.hpp"

#include "SDL3/SDL_video.h"

#include <alloca.h>
#include <filesystem>
#include <string>
#include <vector>

static Uint64 lastTime = SDL_GetPerformanceCounter();

void GlClearError()
{
    while (glGetError() != GL_NO_ERROR)
        ;
}

bool GlLogCall(const char* function, const char* file, unsigned int line)
{
    while (GLenum error = glGetError())
    {
        std::println(stderr, "[OpenGl Error] ({}): {} :{} {}", error, function, file, line);
        return false;
    }
    return true;
}

RendererOpenGL::RendererOpenGL(Window& window) : _window(window)
{
    _GLContext = SDL_GL_CreateContext(window.GetSDLWindow());

    if (!_GLContext)
    {
        throw std::runtime_error(SDL_GetError());
    }

    SDL_GL_MakeCurrent(_window.GetSDLWindow(), _GLContext);

    // Limiting frame rate (VSync).
    SDL_GL_SetSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
    {
        throw std::runtime_error(SDL_GetError());
    }

    std::println("Using Renderer: {} {}", reinterpret_cast<const char*>(glGetString(GL_RENDERER)),
                 reinterpret_cast<const char*>(glGetString(GL_VERSION)));
}

RendererOpenGL::~RendererOpenGL()
{
    SDL_GL_DestroyContext(_GLContext);
}

Matrix4 RendererOpenGL::GetFinalMatrix(RotationAxis activeAxis, const Matrix4& accumulatedRotationMatrix)
{
    Matrix4 rotationMatrix = Matrix4(1.0f);

    switch (activeAxis)
    {
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

void RendererOpenGL::Start()
{
    if (!_mesh)
    {
        throw std::runtime_error("Renderer: model not set");
    }

    if (!_texture)
    {
        throw std::runtime_error("Renderer: texture not set");
    }

    if (!_noiseTexture)
    {
        throw std::runtime_error("Renderer: noise texture not set");
    }

    GlCall(glEnable(GL_CULL_FACE));
    GlCall(glEnable(GL_DEPTH_TEST));
    GlCall(glDepthFunc(GL_LESS));
    GlCall(glEnable(GL_BLEND));
    GlCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    GlCall(glGenVertexArrays(1, &_VAO));
    GlCall(glBindVertexArray(_VAO));

    _vb = std::make_unique<VertexBuffer>(_mesh->_vertexBuffer.data(), _mesh->_vertexBuffer.size() * sizeof(Vertex));
    _ib = std::make_unique<IndexBuffer>(_mesh->_verticesIndices.data(), _mesh->_verticesIndices.size());

    // Set up vertex attribute pointers. The first 3 float are the positions.
    // Vertex positions (attribute 0) on shader.
    GlCall(glEnableVertexAttribArray(0));
    GlCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const void*>(offsetof(Vertex, position))));

    // The next 2 float are the texture coordinates.
    // Texture coordinates (attribute 1) on shader.
    GlCall(glEnableVertexAttribArray(1));
    GlCall(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const void*>(offsetof(Vertex, texCoord))));

    _texture->Bind();
    _noiseTexture->Bind(1);

    _projectionMatrix = Matrix4::perspective(45.0f, _window.GetWindowWidth(), _window.GetWindowHeight(), 0.1f, 1000.0f);
    _translateToOrigin = Matrix4::translation(-_mesh->_pivot);
    _translateBack = Matrix4::translation(_mesh->_pivot);

    const std::filesystem::path shaderDirectory = std::filesystem::path(ASSET_DIR) / "shaders/glsl";
    _textureShader = std::make_unique<ShaderOpenGL>(shaderDirectory / "Basic.glsl");
    _dissolveShader = std::make_unique<ShaderOpenGL>(shaderDirectory / "Dissolve.glsl");

    for (ShaderOpenGL* shader : {_textureShader.get(), _dissolveShader.get()})
    {
        shader->Bind();
        shader->SetUniform1i("u_Texture", 0);
        shader->SetUniformMat4f("u_ProjectionMatrix", _projectionMatrix);
    }

    _dissolveShader->Bind();
    _dissolveShader->SetUniform1i("u_DissolveTexture", 1);
}

void RendererOpenGL::Update(float deltaTime, Camera& camera)
{
    if (_transitioning)
    {
        if (_blendFactor < _targetBlendFactor)
        {
            _blendFactor += BLEND_SPEED;
            if (_blendFactor >= _targetBlendFactor)
            {
                _blendFactor = _targetBlendFactor;
                _transitioning = false;
            }
        }
        else if (_blendFactor > _targetBlendFactor)
        {
            _blendFactor -= BLEND_SPEED;
            if (_blendFactor <= _targetBlendFactor)
            {
                _blendFactor = _targetBlendFactor;
                _transitioning = false;
            }
        }
    }

    if (_useDissolve)
    {
        _dissolveAmount += deltaTime * 0.75f;
        if (_dissolveAmount > 1.0f)
        {
            _dissolveAmount = 1.0f;
        }
    }
    else
    {
        _dissolveAmount -= deltaTime * 0.75f;
        if (_dissolveAmount < 0.0f)
        {
            _dissolveAmount = 0.0f;
        }
    }

    _viewMatrix = Matrix4::rotationY(camera.rotationAngle) * Matrix4::translation(-camera.pos);
    _accumulatedRotationMatrix = GetFinalMatrix(_activeAxis, _accumulatedRotationMatrix);
}

void RendererOpenGL::Render()
{
    GlCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    Matrix4 modelMatrix = _translateToOrigin * _accumulatedRotationMatrix * _translateBack;

    // Keep the dissolve shader active until the reverse transition finishes.
    const bool dissolving = _useDissolve || _dissolveAmount > 0.0f;
    ShaderOpenGL& shader = dissolving ? *_dissolveShader : *_textureShader;
    shader.Bind();

    _texture->Bind(0);
    if (dissolving)
    {
        _noiseTexture->Bind(1);
        shader.SetUniform1f("u_DissolveAmount", _dissolveAmount);
    }
    shader.SetUniform1f("u_ModeFactor", _blendFactor);
    shader.SetUniformMat4f("u_ViewMatrix", _viewMatrix);
    shader.SetUniformMat4f("u_ModelMatrix", modelMatrix);

    GlCall(glBindVertexArray(_VAO));
    _ib->Bind();
    GlCall(glDrawElements(GL_TRIANGLES, _mesh->_verticesIndices.size(), GL_UNSIGNED_INT, nullptr));
}

void RendererOpenGL::SwapBuffers()
{
    SDL_GL_SwapWindow(_window.GetSDLWindow());
}

void RendererOpenGL::SetPolygonMode(RenderMode mode)
{
    switch (mode)
    {
    case RenderMode::POINT:
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
        break;
    case RenderMode::LINE:
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        break;
    case RenderMode::FILL:
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        break;
    }
}
