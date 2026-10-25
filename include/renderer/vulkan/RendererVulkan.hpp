#pragma once

#include <SDL3/SDL.h>
#include <cstddef>
#include <cstdint>
#include <glad/glad.h>
#include <memory>
#include <optional>
#include <vector>
#include <vulkan/vulkan_raii.hpp>

#include "ITexture.hpp"
#include "Mesh.hpp"
#include "Window.hpp"
#include "camera.hpp"
#include "math/Matrix4.hpp"
#include "renderer/IRenderer.hpp"
#include "renderer/vulkan/TextureVulkan.hpp"
#include "vulkan/vulkan.hpp"

constexpr float CAMERA_SPEED = 10.0f;
constexpr float ROTATION_SPEED = 0.5f;
constexpr float BLEND_SPEED = 0.02f;
constexpr int MAX_FRAMES_IN_FLIGHT = 2;

class RendererVulkan : public IRenderer
{

    public:
    RendererVulkan() = delete;
    explicit RendererVulkan(Window& window);
    ~RendererVulkan();

    void Start() override;
    void Update(float deltaTime, Camera& camera) override;
    void Render() override;
    void OnWindowResized() override
    {
        _framebufferResized = true;
    }

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
        return std::make_unique<TextureVulkan>(path);
    }

    Matrix4 GetFinalMatrix(RotationAxis activeAxis, const Matrix4& accumulatedRotationMatrix);

    private:
    static vk::SurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
    static vk::PresentModeKHR ChooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);
    [[nodiscard]] vk::raii::ShaderModule CreateShaderModule(const std::vector<char>& code) const;
    vk::Extent2D ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);
    static uint32_t ChooseSwapMinImageCount(const vk::SurfaceCapabilitiesKHR& surfaceCapabilities);
    void CreateInstance();
    void SetupDebugMessenger();
    std::optional<uint32_t> FindGraphicsPresentQueueFamily(const vk::raii::PhysicalDevice& device);
    bool IsDeviceSuitable(const vk::raii::PhysicalDevice& device);
    void PickPhysicalDevice();
    void CreateLogicalDevice();
    void CreateSurface();
    void CreateSwapChain();
    vk::raii::ImageView CreateImageView(const vk::Image& image, vk::Format format, vk::ImageAspectFlags aspectFlags);
    void CreateImageViews();
    void CreateDescriptorSetLayout();
    void CreateGraphicsPipeline();
    void CreateCommandPool();
    void CreateDepthResources();
    void CreateTextureImage();
    void CreateTextureImageView();
    void CreateTextureSampler();
    void CreateCommandBuffers();
    void CreateSyncObjects();
    void CreateVertexBuffer();
    void CreateIndexBuffer();
    void CreateUniformBuffers();
    void CreateDescriptorPool();
    void CreateDescriptorSets();
    std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> CreateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage,
                                                                     vk::MemoryPropertyFlags properties);
    std::pair<vk::raii::Image, vk::raii::DeviceMemory> CreateImage(uint32_t width, uint32_t height, vk::Format format,
                                                                   vk::ImageTiling tiling, vk::ImageUsageFlags usage,
                                                                   vk::MemoryPropertyFlags properties);

    void RecordCommandBuffer(uint32_t imageIndex);
    void TransitionImageLayout(vk::Image image, vk::ImageLayout old_layout, vk::ImageLayout new_layout,
                               vk::AccessFlags2 src_access_mask, vk::AccessFlags2 dst_access_mask,
                               vk::PipelineStageFlags2 src_stage_mask, vk::PipelineStageFlags2 dst_stage_mask,
                               vk::ImageAspectFlags aspectFlags);
    void TransitionImageLayout(vk::raii::CommandBuffer& commandBuffer, const vk::raii::Image& image,
                               vk::ImageLayout oldLayout, vk::ImageLayout newLayout);
    void RecreateSwapChain();
    void CleanupSwapChain();

    uint32_t FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
    vk::Format FindSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling,
                                   vk::FormatFeatureFlags features);
    vk::Format FindDepthFormat();

    void CopyBuffer(vk::raii::Buffer& srcBuffer, vk::raii::Buffer& dstBuffer, vk::DeviceSize size);
    void CopyBufferToImage(vk::raii::CommandBuffer& commandBuffer, const vk::raii::Buffer& buffer,
                           vk::raii::Image& image, uint32_t width, uint32_t height);

    void UpdateUniformBuffer(uint32_t currentImage);

    vk::raii::CommandBuffer BeginSingleTimeCommands();
    void EndSingleTimeCommands(vk::raii::CommandBuffer&& commandBuffer);

#ifndef NDEBUG
    static vk::DebugUtilsMessengerCreateInfoEXT CreateDebugMessengerInfo();
    static VKAPI_ATTR vk::Bool32 VKAPI_CALL DebugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
                                                          vk::DebugUtilsMessageTypeFlagsEXT type,
                                                          const vk::DebugUtilsMessengerCallbackDataEXT* callbackData,
                                                          void* userData);
#endif
    vk::raii::Context _context;
    vk::raii::Instance _instance{nullptr};
#ifndef NDEBUG
    vk::raii::DebugUtilsMessengerEXT _debugMessenger{nullptr};
#endif
    vk::raii::SurfaceKHR _surface{nullptr};
    vk::raii::PhysicalDevice _physicalDevice{nullptr};
    vk::raii::Device _device{nullptr};
    vk::raii::Queue _queue{nullptr};
    uint32_t _queueFamilyIndex = 0;
    vk::raii::Queue _transferQueue{nullptr};
    uint32_t _transferQueueFamilyIndex = 0;
    vk::Extent2D _swapChainExtent{};
    vk::SurfaceFormatKHR _swapChainSurfaceFormat{};
    vk::raii::SwapchainKHR _swapChain{nullptr};
    std::vector<vk::Image> _swapChainImages{};
    std::vector<vk::raii::ImageView> _swapChainImageViews{};
    vk::raii::DescriptorSetLayout _descriptorSetLayout{nullptr};
    vk::raii::PipelineLayout _pipelineLayout{nullptr};
    vk::raii::Pipeline _graphicsPipeline{nullptr};
    vk::raii::CommandPool _commandPool{nullptr};
    vk::raii::CommandPool _transferCommandPool{nullptr};
    std::vector<vk::raii::CommandBuffer> _commandBuffers{};
    std::vector<vk::raii::Semaphore> _presentCompleteSemaphores{};
    std::vector<vk::raii::Semaphore> _renderFinishedSemaphores{};
    std::vector<vk::raii::Fence> _inFlightFences{};
    uint32_t _frameIndex = 0;
    bool _framebufferResized = false;
    vk::raii::DeviceMemory _vertexBufferMemory{nullptr};
    vk::raii::Buffer _vertexBuffer{nullptr};
    vk::raii::DeviceMemory _indexBufferMemory{nullptr};
    vk::raii::Buffer _indexBuffer{nullptr};
    std::vector<vk::raii::Buffer> _uniformBuffers{};
    std::vector<vk::raii::DeviceMemory> _uniformBuffersMemory{};
    std::vector<void*> _uniformBuffersMapped{};
    vk::raii::DescriptorPool _descriptorPool{nullptr};
    std::vector<vk::raii::DescriptorSet> _descriptorSets{};
    vk::raii::DeviceMemory _textureImageMemory{nullptr};
    vk::raii::Image _textureImage{nullptr};
    vk::raii::ImageView _textureImageView{nullptr};
    vk::raii::Sampler _textureSampler{nullptr};
    vk::raii::DeviceMemory _depthImageMemory{nullptr};
    vk::raii::Image _depthImage{nullptr};
    vk::raii::ImageView _depthImageView{nullptr};

    Matrix4 _translateToOrigin;
    Matrix4 _translateBack;
    Matrix4 _accumulatedRotationMatrix = Matrix4(1.0f);
    Matrix4 _viewMatrix;
    Matrix4 _projectionMatrix;

    RotationAxis _activeAxis = RotationAxis::NONE;

    Window& _window;

    std::unique_ptr<ITexture> _texture;
    std::unique_ptr<ITexture> _noiseTexture;

    std::unique_ptr<Mesh> _mesh;

    float _dissolveAmount = 0.0f;
    float _blendFactor = 0.0f;
    float _targetBlendFactor = 0.0f;

    bool _useDissolve = false;
    bool _transitioning = false;
};
