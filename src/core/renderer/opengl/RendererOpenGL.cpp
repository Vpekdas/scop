#include "renderer/opengl/RendererOpenGL.hpp"
#include "renderer/opengl/IndexBuffer.hpp"
#include "renderer/opengl/ShaderOpenGL.hpp"
#include "renderer/opengl/TextureOpenGL.hpp"

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
        std::cerr << "[OpenGl Error] (" << error << "): " << function << " " << ":" << file << " " << line << "\n";
        return false;
    }
    return true;
}

RendererOpenGL::RendererOpenGL(Window& window) : _window(window)
{
    _badAppleFrames.resize(6572);

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

    std::cout << "Using Renderer: " << glGetString(GL_RENDERER) << " " << glGetString(GL_VERSION) << "\n";
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

// https://stackoverflow.com/questions/6495523/ffmpeg-video-to-opengl-texture
void RendererOpenGL::LoadFrameIfNeeded(std::size_t frameIndex)
{
    if (!_badAppleFrames[frameIndex])
    {
        std::filesystem::path path =
            std::filesystem::path(ASSET_DIR) / "textures" / "bad_apple" / (std::to_string(frameIndex + 1) + ".tga");

        _badAppleFrames[frameIndex] = std::make_unique<TextureOpenGL>(path);
    }
}

void RendererOpenGL::Start()
{
    if (!_model)
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

    _vb = std::make_unique<VertexBuffer>(_model->_vertexBuffer.data(), _model->_vertexBuffer.size() * sizeof(float));
    _ib = std::make_unique<IndexBuffer>(_model->_verticesIndices.data(), _model->_verticesIndices.size());

    // Set up vertex attribute pointers. The first 3 float are the positions.
    // Vertex positions (attribute 0) on shader.
    GlCall(glEnableVertexAttribArray(0));
    GlCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (const void*)(0)));

    // The next 2 float are the texture coordinates.
    // Texture coordinates (attribute 1) on shader.
    GlCall(glEnableVertexAttribArray(1));
    GlCall(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (const void*)(3 * sizeof(float))));

    _texture->Bind();
    _noiseTexture->Bind(1);

    _projectionMatrix = Matrix4::perspective(45.0f, _window.GetWindowWidth(), _window.GetWindowHeight(), 0.1f, 1000.0f);
    _translateToOrigin = Matrix4::translation(-_model->_centroid);
    _translateBack = Matrix4::translation(_model->_centroid);

    std::filesystem::path shaderPath = std::filesystem::path(ASSET_DIR) / "shader" / "Basic.glsl";
    _shader = std::make_unique<ShaderOpenGL>(shaderPath);
    _shader->Bind();
    _shader->SetUniform1i("u_Texture", 0);
    _shader->SetUniform1i("u_DissolveTexture", 0);
    _shader->SetUniformMat4f("u_ViewMatrix", _viewMatrix);
    _shader->SetUniformMat4f("u_ProjectionMatrix", _projectionMatrix);

    std::filesystem::path quadShaderPath = std::filesystem::path(ASSET_DIR) / "shader" / "Quad.glsl";
    _quadShader = std::make_unique<ShaderOpenGL>(quadShaderPath);
    _quadShader->Bind();
    _quadShader->SetUniform1i("u_Texture", 0);

    const float quadVertices[] = {-1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,  -1.0f, 0.0f, 1.0f, 1.0f,
                                  1.0f,  1.0f,  0.0f, 1.0f, 0.0f, -1.0f, 1.0f,  0.0f, 0.0f, 0.0f};

    const unsigned int quadIndices[] = {0, 1, 2, 2, 3, 0};

    GlCall(glGenVertexArrays(1, &_quadVAO));
    GlCall(glBindVertexArray(_quadVAO));

    _quadVB = std::make_unique<VertexBuffer>(quadVertices, sizeof(quadVertices));
    _quadIB = std::make_unique<IndexBuffer>(quadIndices, 6);

    GlCall(glEnableVertexAttribArray(0));
    GlCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0));

    GlCall(glEnableVertexAttribArray(1));
    GlCall(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float))));

    LoadFrameIfNeeded(_currentFrame);
}

void RendererOpenGL::Update(float deltaTime, Camera& camera)
{
    _frameTimer += deltaTime;

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

    while (_frameTimer >= FRAME_TIME)
    {
        _frameTimer -= FRAME_TIME;

        _currentFrame++;

        if (_currentFrame >= _badAppleFrames.size())
        {
            _currentFrame = 0;
        }

        LoadFrameIfNeeded(_currentFrame);
    }

    _viewMatrix = Matrix4::rotationY(camera.rotationAngle) * Matrix4::translation(-camera.pos);
    _accumulatedRotationMatrix = GetFinalMatrix(_activeAxis, _accumulatedRotationMatrix);
}

void RendererOpenGL::Render()
{

    if (_useBadAppleOnModel)
    {
        glClearColor(0.376f, 0.647f, 0.980f, 1.0f);
    }

    GlCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    Matrix4 modelMatrix = _translateToOrigin * _accumulatedRotationMatrix * _translateBack;

    if (!_useBadAppleOnModel)
    {
        GlCall(glDisable(GL_DEPTH_TEST));
        GlCall(glBindVertexArray(_quadVAO));
        _quadShader->Bind();
        _badAppleFrames[_currentFrame]->Bind();
        GlCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));
        GlCall(glEnable(GL_DEPTH_TEST));
    }

    _shader->Bind();

    if (_useBadAppleOnModel)
    {
        _badAppleFrames[_currentFrame]->Bind();
    }
    else
    {
        _texture->Bind();
    }

    _shader->SetUniform1f("u_ModeFactor", _blendFactor);
    _shader->SetUniform1f("u_DissolveAmount", _dissolveAmount);
    _shader->SetUniformMat4f("u_ViewMatrix", _viewMatrix);
    _shader->SetUniformMat4f("u_ModelMatrix", modelMatrix);

    GlCall(glBindVertexArray(_VAO));
    _ib->Bind();
    GlCall(glDrawElements(GL_TRIANGLES, _model->_verticesIndices.size(), GL_UNSIGNED_INT, nullptr));
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
