#pragma once

#include "AudioPlayer.hpp"
#include "Model.hpp"
#include "Window.hpp"
#include "camera.hpp"
#include "renderer/IRenderer.hpp"
#include <memory>

constexpr double BAD_APPLE_FPS = 30.0;

class Model;

class Application
{
    public:
    Application() = delete;
    explicit Application(uint32_t width, uint32_t height, const char* title);
    ~Application();

    void Run();
    void ProcessInput() {};

    inline void LoadModel(const std::string& path)
    {
        _renderer->LoadModel(std::make_unique<Model>(path));
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
    std::unique_ptr<AudioPlayer> _audioPlayer;
    Camera _camera;

    bool _isRunning = true;
};