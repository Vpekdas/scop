#pragma once

#include "Mesh.hpp"
#include "Window.hpp"
#include "camera.hpp"
#include "renderer/IRenderer.hpp"
#include <memory>

class Mesh;

class Application
{
    public:
    Application() = delete;
    explicit Application(uint32_t width, uint32_t height, const char* title);
    ~Application();

    void Run();
    void ProcessInput() {};

    inline void LoadMesh(const std::string& path)
    {
        _renderer->LoadMesh(std::make_unique<Mesh>(path));
    }

    inline void LoadTexture(const std::string& path)
    {
        std::unique_ptr<ITexture> tex = _renderer->CreateTexture(path);
        _renderer->LoadTexture(std::move(tex));
    }

    inline void LoadNoiseTexture(const std::string& path)
    {
        std::unique_ptr<ITexture> tex = _renderer->CreateTexture(path);
        _renderer->LoadNoiseTexture(std::move(tex));
    }

    private:
    std::unique_ptr<Window> _window;
    std::unique_ptr<IRenderer> _renderer;
    Camera _camera;

    bool _isRunning = true;
};