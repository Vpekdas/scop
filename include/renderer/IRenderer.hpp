#pragma once

#include "ITexture.hpp"
#include "camera.hpp"

enum class RotationAxis
{
    NONE,
    X,
    Y,
    Z
};

enum class RenderMode
{
    POINT,
    LINE,
    FILL
};

class ITexture;
class Mesh;

class IRenderer
{
    public:
    IRenderer() = default;
    virtual ~IRenderer() = default;

    virtual void Start() = 0;
    virtual void Update(float deltaTime, Camera& camera) = 0;
    virtual void Render() = 0;
    virtual void OnWindowResized() {}

    virtual void SetRotationAxis(RotationAxis) = 0;
    virtual void SetPolygonMode(RenderMode) = 0;
    virtual void ApplyTexture() = 0;
    virtual void ApplyDissolve() = 0;

    virtual void SwapBuffers() = 0;
    virtual void LoadMesh(std::unique_ptr<Mesh>) = 0;
    virtual void LoadTexture(std::unique_ptr<ITexture>) = 0;
    virtual void LoadNoiseTexture(std::unique_ptr<ITexture>) = 0;

    virtual std::unique_ptr<ITexture> CreateTexture(const std::string& path) = 0;
};
