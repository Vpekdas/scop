#include "SDL3/SDL_vulkan.h"
#include "UniformBufferObject.hpp"
#include "camera.hpp"
#include "math/vector.hpp"

#include "SDL3/SDL_video.h"

#include "renderer/vulkan/RendererVulkan.hpp"
#include "renderer/vulkan/VertexInput.hpp"
#include <algorithm>
#include <alloca.h>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <limits>
#include <map>
#include <optional>
#include <print>
#include <set>
#include <stdexcept>
#include <vector>

#include <SDL3/SDL_vulkan.h>
#include <fstream>
#include <vulkan/vulkan_raii.hpp>

static std::vector<char> readFile(const std::string& filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open file: " + filename);
    }
    std::vector<char> buffer(file.tellg());
    file.seekg(0, std::ios::beg);
    file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
    file.close();

    return buffer;
}

#ifndef NDEBUG
VKAPI_ATTR vk::Bool32
    VKAPI_CALL RendererVulkan::DebugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT,
                                             vk::DebugUtilsMessageTypeFlagsEXT type,
                                             const vk::DebugUtilsMessengerCallbackDataEXT* callbackData, void*)
{
    std::println(stderr, "Vulkan validation [{}]: {}", vk::to_string(type), callbackData->pMessage);
    return vk::False;
}
#endif

RendererVulkan::RendererVulkan(Window& window) : _window(window)
{
    CreateInstance();
    SetupDebugMessenger();
    CreateSurface();
    PickPhysicalDevice();
    CreateLogicalDevice();
    CreateSwapChain();
    CreateImageViews();
    CreateDescriptorSetLayout();
    CreateGraphicsPipeline();
    CreateCommandPool();
    CreateCommandBuffers();
    CreateUniformBuffers();
    CreateDescriptorPool();
    CreateSyncObjects();
}

void RendererVulkan::CreateInstance()
{
    constexpr vk::ApplicationInfo appInfo{.pApplicationName = "scop",
                                          .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
                                          .pEngineName = "No Engine",
                                          .engineVersion = VK_MAKE_VERSION(1, 0, 0),
                                          .apiVersion = vk::ApiVersion14};

    Uint32 extensionCount = 0;
    auto extensions = SDL_Vulkan_GetInstanceExtensions(&extensionCount);
    if (!extensions)
    {
        throw std::runtime_error(SDL_GetError());
    }
    std::vector<const char*> requiredExtensions(extensions, extensions + extensionCount);

#ifndef NDEBUG
    requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    const char* validationLayer = "VK_LAYER_KHRONOS_validation";
    auto layers = _context.enumerateInstanceLayerProperties();
    if (std::ranges::none_of(layers, [validationLayer](const auto& layer)
                             { return std::strcmp(layer.layerName.data(), validationLayer) == 0; }))
    {
        throw std::runtime_error("Required Vulkan layer unavailable: VK_LAYER_KHRONOS_validation");
    }
    auto debugInfo = CreateDebugMessengerInfo();
#endif

    vk::InstanceCreateInfo createInfo{
#ifndef NDEBUG
        .pNext = &debugInfo,
#endif
        .pApplicationInfo = &appInfo,
#ifndef NDEBUG
        .enabledLayerCount = 1,
        .ppEnabledLayerNames = &validationLayer,
#endif
        .enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size()),
        .ppEnabledExtensionNames = requiredExtensions.data()};

    _instance = vk::raii::Instance(_context, createInfo);
}

#ifndef NDEBUG
vk::DebugUtilsMessengerCreateInfoEXT RendererVulkan::CreateDebugMessengerInfo()
{
    return {.messageSeverity =
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
            .messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                           vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                           vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
            .pfnUserCallback = &RendererVulkan::DebugCallback};
}
#endif

void RendererVulkan::SetupDebugMessenger()
{
#ifndef NDEBUG
    _debugMessenger = _instance.createDebugUtilsMessengerEXT(CreateDebugMessengerInfo());
#endif
}

std::optional<uint32_t> RendererVulkan::FindGraphicsPresentQueueFamily(const vk::raii::PhysicalDevice& device)
{
    const auto families = device.getQueueFamilyProperties();
    for (uint32_t i = 0; i < families.size(); ++i)
    {
        if (families[i].queueCount > 0 && bool(families[i].queueFlags & vk::QueueFlagBits::eGraphics) &&
            device.getSurfaceSupportKHR(i, *_surface))
        {
            return i;
        }
    }
    return std::nullopt;
}

bool RendererVulkan::IsDeviceSuitable(const vk::raii::PhysicalDevice& device)
{
    if (device.getProperties().apiVersion < vk::ApiVersion14)
    {
        return false;
    }

    if (!FindGraphicsPresentQueueFamily(device))
    {
        return false;
    }

    const auto extensions = device.enumerateDeviceExtensionProperties();
    const bool supportsSwapchain = std::ranges::any_of(
        extensions, [](const auto& extension)
        { return std::strcmp(extension.extensionName.data(), vk::KHRSwapchainExtensionName) == 0; });
    if (!supportsSwapchain)
    {
        return false;
    }

    const auto features = device.getFeatures2<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan11Features>();
    return features.get<vk::PhysicalDeviceFeatures2>().features.samplerAnisotropy &&
           features.get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters;
}

void RendererVulkan::PickPhysicalDevice()
{
    auto physicalDevices = vk::raii::PhysicalDevices(_instance);
    if (physicalDevices.empty())
    {
        throw std::runtime_error("Failed to find GPUs with Vulkan support !");
    }

    std::multimap<int, vk::raii::PhysicalDevice> candidates;

    for (const auto& pd : physicalDevices)
    {
        if (!IsDeviceSuitable(pd))
        {
            continue;
        }

        const auto deviceProperties = pd.getProperties();
        uint32_t score = 0;

        if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
        {
            score += 1000;
        }

        score += deviceProperties.limits.maxImageDimension2D;
        candidates.insert(std::make_pair(score, pd));
    }

    if (!candidates.empty() && candidates.rbegin()->first > 0)
    {
        _physicalDevice = candidates.rbegin()->second;
    }
    else
    {
        throw std::runtime_error("Failed to find a suitable GPU !");
    }
}

void RendererVulkan::CreateLogicalDevice()
{
    const auto queueFamily = FindGraphicsPresentQueueFamily(_physicalDevice);
    if (!queueFamily)
    {
        throw std::runtime_error("No queue family supports both graphics and presentation.");
    }
    _queueFamilyIndex = *queueFamily;

    _transferQueueFamilyIndex = _queueFamilyIndex;
    const auto families = _physicalDevice.getQueueFamilyProperties();
    for (uint32_t i = 0; i < families.size(); ++i)
    {
        if (families[i].queueCount > 0 && bool(families[i].queueFlags & vk::QueueFlagBits::eTransfer) &&
            !(families[i].queueFlags & vk::QueueFlagBits::eGraphics))
        {
            _transferQueueFamilyIndex = i;
            if (!(families[i].queueFlags & vk::QueueFlagBits::eCompute))
            {
                break;
            }
        }
    }

    const float queuePriority = 0.5f;
    const std::set<uint32_t> uniqueFamilies{_queueFamilyIndex, _transferQueueFamilyIndex};
    std::vector<vk::DeviceQueueCreateInfo> queueInfos;
    for (uint32_t family : uniqueFamilies)
    {
        queueInfos.push_back({.queueFamilyIndex = family, .queueCount = 1, .pQueuePriorities = &queuePriority});
    }

    vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan11Features,
                       vk::PhysicalDeviceVulkan12Features, vk::PhysicalDeviceVulkan13Features>
        featureChain = {
            {.features = {.samplerAnisotropy = true}}, // Enable anisotropic filtering
            {.shaderDrawParameters = true},            // Enable shader draw parameters from Vulkan 1.1
            {.separateDepthStencilLayouts = true},
            {.synchronization2 = true, .dynamicRendering = true} // Enable rendering and image barriers
    };

    std::vector<const char*> requiredDeviceExtension = {vk::KHRSwapchainExtensionName};

    vk::DeviceCreateInfo deviceCreateInfo{.pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
                                          .queueCreateInfoCount = static_cast<uint32_t>(queueInfos.size()),
                                          .pQueueCreateInfos = queueInfos.data(),
                                          .enabledExtensionCount =
                                              static_cast<uint32_t>(requiredDeviceExtension.size()),
                                          .ppEnabledExtensionNames = requiredDeviceExtension.data()};

    _device = vk::raii::Device(_physicalDevice, deviceCreateInfo);
    _queue = vk::raii::Queue(_device, _queueFamilyIndex, 0);
    _transferQueue = vk::raii::Queue(_device, _transferQueueFamilyIndex, 0);
    std::println("Queue families: graphics/present {}, transfer {}", _queueFamilyIndex, _transferQueueFamilyIndex);
}

vk::SurfaceFormatKHR RendererVulkan::ChooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats)
{
    const auto formatIt = std::ranges::find_if(availableFormats,
                                               [](const auto& format)
                                               {
                                                   return format.format == vk::Format::eB8G8R8A8Srgb &&
                                                          format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;
                                               });
    return formatIt != availableFormats.end() ? *formatIt : availableFormats[0];
}

vk::PresentModeKHR RendererVulkan::ChooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes)
{
    assert(std::ranges::any_of(availablePresentModes,
                               [](auto presentMode) { return presentMode == vk::PresentModeKHR::eFifo; }));

    return std::ranges::any_of(availablePresentModes,
                               [](auto presentMode) { return presentMode == vk::PresentModeKHR::eMailbox; })
               ? vk::PresentModeKHR::eMailbox
               : vk::PresentModeKHR::eFifo;
}

[[nodiscard]] vk::raii::ShaderModule RendererVulkan::CreateShaderModule(const std::vector<char>& code) const
{
    vk::ShaderModuleCreateInfo createInfo{.codeSize = code.size() * sizeof(char),
                                          .pCode = reinterpret_cast<const uint32_t*>(code.data())};
    vk::raii::ShaderModule shaderModule{_device, createInfo};
    return shaderModule;
}

vk::Extent2D RendererVulkan::ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }

    int width = 0;
    int height = 0;
    if (!SDL_GetWindowSizeInPixels(_window.GetSDLWindow(), &width, &height))
    {
        throw std::runtime_error(SDL_GetError());
    }

    return {
        std::clamp(static_cast<uint32_t>(width), capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
        std::clamp(static_cast<uint32_t>(height), capabilities.minImageExtent.height,
                   capabilities.maxImageExtent.height)};
}

uint32_t RendererVulkan::ChooseSwapMinImageCount(const vk::SurfaceCapabilitiesKHR& surfaceCapabilities)
{
    auto minImageCount = std::max(3u, surfaceCapabilities.minImageCount);
    if (surfaceCapabilities.maxImageCount > 0 && surfaceCapabilities.maxImageCount < minImageCount)
    {
        minImageCount = surfaceCapabilities.maxImageCount;
    }
    return minImageCount;
}

void RendererVulkan::CreateSurface()
{
    VkSurfaceKHR surface = VK_NULL_HANDLE;

    if (!SDL_Vulkan_CreateSurface(_window.GetSDLWindow(), *_instance, nullptr, &surface))
    {
        throw std::runtime_error(SDL_GetError());
    }

    _surface = vk::raii::SurfaceKHR(_instance, surface);
}
void RendererVulkan::CreateSwapChain()
{
    vk::SurfaceCapabilitiesKHR surfaceCapabilities = _physicalDevice.getSurfaceCapabilitiesKHR(*_surface);
    _swapChainExtent = ChooseSwapExtent(surfaceCapabilities);
    uint32_t minImageCount = ChooseSwapMinImageCount(surfaceCapabilities);

    std::vector<vk::SurfaceFormatKHR> availableFormats = _physicalDevice.getSurfaceFormatsKHR(*_surface);
    _swapChainSurfaceFormat = ChooseSwapSurfaceFormat(availableFormats);

    auto availablePresentModes = _physicalDevice.getSurfacePresentModesKHR(*_surface);

    vk::SwapchainCreateInfoKHR swapChainCreateInfo{.surface = *_surface,
                                                   .minImageCount = minImageCount,
                                                   .imageFormat = _swapChainSurfaceFormat.format,
                                                   .imageColorSpace = _swapChainSurfaceFormat.colorSpace,
                                                   .imageExtent = _swapChainExtent,
                                                   .imageArrayLayers = 1,
                                                   .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
                                                   .imageSharingMode = vk::SharingMode::eExclusive,
                                                   .preTransform = surfaceCapabilities.currentTransform,
                                                   .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
                                                   .presentMode = ChooseSwapPresentMode(availablePresentModes),
                                                   .clipped = true};

    _swapChain = vk::raii::SwapchainKHR(_device, swapChainCreateInfo);
    _swapChainImages = _swapChain.getImages();
}

vk::raii::ImageView RendererVulkan::CreateImageView(const vk::Image& image, vk::Format format,
                                                    vk::ImageAspectFlags aspectFlags)
{
    const vk::ImageViewCreateInfo viewInfo{
        .image = image,
        .viewType = vk::ImageViewType::e2D,
        .format = format,
        .subresourceRange = {
            .aspectMask = aspectFlags, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1}};
    return vk::raii::ImageView(_device, viewInfo);
}

void RendererVulkan::CreateImageViews()
{
    assert(_swapChainImageViews.empty());
    _swapChainImageViews.reserve(_swapChainImages.size());
    for (const auto& image : _swapChainImages)
    {
        _swapChainImageViews.emplace_back(
            CreateImageView(image, _swapChainSurfaceFormat.format, vk::ImageAspectFlagBits::eColor));
    }
}

void RendererVulkan::CreateDescriptorSetLayout()
{

    std::array<vk::DescriptorSetLayoutBinding, 2> bindings{
        {{.binding = 0,
          .descriptorType = vk::DescriptorType::eUniformBuffer,
          .descriptorCount = 1,
          .stageFlags = vk::ShaderStageFlagBits::eVertex},
         {.binding = 1,
          .descriptorType = vk::DescriptorType::eCombinedImageSampler,
          .descriptorCount = 1,
          .stageFlags = vk::ShaderStageFlagBits::eFragment}}};

    vk::DescriptorSetLayoutCreateInfo layoutInfo{.bindingCount = static_cast<uint32_t>(bindings.size()),
                                                 .pBindings = bindings.data()};

    _descriptorSetLayout = vk::raii::DescriptorSetLayout(_device, layoutInfo);
}

void RendererVulkan::CreateGraphicsPipeline()
{

    const auto shaderPath = std::filesystem::path(ASSET_DIR) / "shaders/spirv/Basic.spv";
    vk::raii::ShaderModule shaderModule = CreateShaderModule(readFile(shaderPath.string()));

    vk::PipelineShaderStageCreateInfo vertShaderStageInfo{
        .stage = vk::ShaderStageFlagBits::eVertex, .module = shaderModule, .pName = "vertMain"};
    vk::PipelineShaderStageCreateInfo fragShaderStageInfo{
        .stage = vk::ShaderStageFlagBits::eFragment, .module = shaderModule, .pName = "fragMain"};
    vk::PipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    std::vector<vk::DynamicState> dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
    vk::PipelineDynamicStateCreateInfo dynamicState{.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
                                                    .pDynamicStates = dynamicStates.data()};

    const auto bindingDescription = GetVertexBindingDescription();
    const auto attributeDescriptions = GetVertexAttributeDescriptions();
    vk::PipelineVertexInputStateCreateInfo vertexInputInfo{
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &bindingDescription,
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
        .pVertexAttributeDescriptions = attributeDescriptions.data()};
    vk::PipelineInputAssemblyStateCreateInfo inputAssembly{.topology = vk::PrimitiveTopology::eTriangleList};
    // vk::Viewport viewport{
    //     0.0f, 0.0f, static_cast<float>(_swapChainExtent.width), static_cast<float>(_swapChainExtent.height),
    //     0.0f, 1.0f};
    // vk::Rect2D scissor{vk::Offset2D{0, 0}, _swapChainExtent};
    vk::PipelineViewportStateCreateInfo viewportState{.viewportCount = 1, .scissorCount = 1};

    vk::PipelineRasterizationStateCreateInfo rasterizer{.depthClampEnable = vk::False,
                                                        .rasterizerDiscardEnable = vk::False,
                                                        .polygonMode = vk::PolygonMode::eFill,
                                                        .cullMode = vk::CullModeFlagBits::eBack,
                                                        .frontFace = vk::FrontFace::eCounterClockwise,
                                                        .depthBiasEnable = vk::False,
                                                        .lineWidth = 1.0f};

    vk::PipelineMultisampleStateCreateInfo multisampling{.rasterizationSamples = vk::SampleCountFlagBits::e1,
                                                         .sampleShadingEnable = vk::False};

    vk::PipelineColorBlendAttachmentState colorBlendAttachment{
        .blendEnable = vk::False,
        .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                          vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA};

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo{
        .setLayoutCount = 1, .pSetLayouts = &*_descriptorSetLayout, .pushConstantRangeCount = 0};
    _pipelineLayout = vk::raii::PipelineLayout(_device, pipelineLayoutInfo);

    vk::PipelineColorBlendStateCreateInfo colorBlending{.logicOpEnable = vk::False,
                                                        .logicOp = vk::LogicOp::eCopy,
                                                        .attachmentCount = 1,
                                                        .pAttachments = &colorBlendAttachment};

    vk::PipelineDepthStencilStateCreateInfo depthStencil{.depthTestEnable = vk::True,
                                                         .depthWriteEnable = vk::True,
                                                         .depthCompareOp = vk::CompareOp::eLess,
                                                         .depthBoundsTestEnable = vk::False,
                                                         .stencilTestEnable = vk::False};

    vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> pipelineCreateInfoChain = {
        {.stageCount = 2,
         .pStages = shaderStages,
         .pVertexInputState = &vertexInputInfo,
         .pInputAssemblyState = &inputAssembly,
         .pViewportState = &viewportState,
         .pRasterizationState = &rasterizer,
         .pMultisampleState = &multisampling,
         .pDepthStencilState = &depthStencil,
         .pColorBlendState = &colorBlending,
         .pDynamicState = &dynamicState,
         .layout = _pipelineLayout,
         .renderPass = nullptr},
        {.colorAttachmentCount = 1,
         .pColorAttachmentFormats = &_swapChainSurfaceFormat.format,
         .depthAttachmentFormat = FindDepthFormat()}};

    _graphicsPipeline =
        vk::raii::Pipeline(_device, nullptr, pipelineCreateInfoChain.get<vk::GraphicsPipelineCreateInfo>());
}

void RendererVulkan::CreateCommandPool()
{
    vk::CommandPoolCreateInfo poolInfo{.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                                       .queueFamilyIndex = _queueFamilyIndex};
    _commandPool = vk::raii::CommandPool(_device, poolInfo);
    const vk::CommandPoolCreateInfo transferInfo{.flags = vk::CommandPoolCreateFlagBits::eTransient,
                                                 .queueFamilyIndex = _transferQueueFamilyIndex};
    _transferCommandPool = vk::raii::CommandPool(_device, transferInfo);
}
void RendererVulkan::CreateDepthResources()
{
    vk::Format depthFormat = FindDepthFormat();
    std::tie(_depthImage, _depthImageMemory) =
        CreateImage(_swapChainExtent.width, _swapChainExtent.height, depthFormat, vk::ImageTiling::eOptimal,
                    vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal);
    _depthImageView = CreateImageView(*_depthImage, depthFormat, vk::ImageAspectFlagBits::eDepth);
}

void RendererVulkan::CreateTextureImage()
{
    const auto* texture = dynamic_cast<const TextureVulkan*>(_texture.get());
    if (!texture)
    {
        throw std::runtime_error("Expected Vulkan texture");
    }

    const auto pixels = texture->GetPixels();
    const vk::DeviceSize imageSize = pixels.size();

    auto [stagingBuffer, stagingBufferMemory] =
        CreateBuffer(imageSize, vk::BufferUsageFlagBits::eTransferSrc,
                     vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    void* data = stagingBufferMemory.mapMemory(0, imageSize);
    std::memcpy(data, pixels.data(), imageSize);
    stagingBufferMemory.unmapMemory();

    std::tie(_textureImage, _textureImageMemory) =
        CreateImage(texture->GetWidth(), texture->GetHeight(), vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal,
                    vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
                    vk::MemoryPropertyFlagBits::eDeviceLocal);

    vk::raii::CommandBuffer commandBuffer = BeginSingleTimeCommands();
    TransitionImageLayout(commandBuffer, _textureImage, vk::ImageLayout::eUndefined,
                          vk::ImageLayout::eTransferDstOptimal);
    CopyBufferToImage(commandBuffer, stagingBuffer, _textureImage, static_cast<uint32_t>(texture->GetWidth()),
                      static_cast<uint32_t>(texture->GetHeight()));

    TransitionImageLayout(commandBuffer, _textureImage, vk::ImageLayout::eTransferDstOptimal,
                          vk::ImageLayout::eShaderReadOnlyOptimal);
    EndSingleTimeCommands(std::move(commandBuffer));
}
void RendererVulkan::CreateTextureImageView()
{
    _textureImageView = CreateImageView(*_textureImage, vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor);
}
void RendererVulkan::CreateTextureSampler()
{
    vk::PhysicalDeviceProperties properties = _physicalDevice.getProperties();
    vk::SamplerCreateInfo samplerInfo{.magFilter = vk::Filter::eLinear,
                                      .minFilter = vk::Filter::eLinear,
                                      .mipmapMode = vk::SamplerMipmapMode::eLinear,
                                      .addressModeU = vk::SamplerAddressMode::eRepeat,
                                      .addressModeV = vk::SamplerAddressMode::eRepeat,
                                      .addressModeW = vk::SamplerAddressMode::eRepeat,
                                      .anisotropyEnable = vk::True,
                                      .maxAnisotropy = properties.limits.maxSamplerAnisotropy,
                                      .compareEnable = vk::False,
                                      .compareOp = vk::CompareOp::eAlways};

    samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
    samplerInfo.unnormalizedCoordinates = vk::False;
    samplerInfo.compareEnable = vk::False;

    _textureSampler = vk::raii::Sampler(_device, samplerInfo);
};

void RendererVulkan::CreateCommandBuffers()
{
    vk::CommandBufferAllocateInfo allocInfo{.commandPool = _commandPool,
                                            .level = vk::CommandBufferLevel::ePrimary,
                                            .commandBufferCount = MAX_FRAMES_IN_FLIGHT};
    _commandBuffers = vk::raii::CommandBuffers(_device, allocInfo);
}
void RendererVulkan::CreateSyncObjects()
{
    assert(_presentCompleteSemaphores.empty() && _renderFinishedSemaphores.empty() && _inFlightFences.empty());

    for (size_t i = 0; i < _swapChainImages.size(); i++)
    {
        _renderFinishedSemaphores.emplace_back(_device, vk::SemaphoreCreateInfo());
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        _presentCompleteSemaphores.emplace_back(_device, vk::SemaphoreCreateInfo());
        _inFlightFences.emplace_back(_device, vk::FenceCreateInfo{.flags = vk::FenceCreateFlagBits::eSignaled});
    }
}

void RendererVulkan::CreateVertexBuffer()
{
    const auto& vertices = _mesh->_vertexBuffer;
    if (vertices.empty())
    {
        throw std::runtime_error("Mesh has no vertices");
    }
    const vk::DeviceSize bufferSize = sizeof(Vertex) * vertices.size();

    auto [stagingBuffer, stagingBufferMemory] =
        CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
                     vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    void* dataStaging = stagingBufferMemory.mapMemory(0, bufferSize);
    memcpy(dataStaging, vertices.data(), bufferSize);
    stagingBufferMemory.unmapMemory();

    std::tie(_vertexBuffer, _vertexBufferMemory) =
        CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
                     vk::MemoryPropertyFlagBits::eDeviceLocal);

    CopyBuffer(stagingBuffer, _vertexBuffer, bufferSize);
}

void RendererVulkan::CreateIndexBuffer()
{
    const auto& indices = _mesh->_verticesIndices;
    if (indices.empty())
    {
        throw std::runtime_error("Mesh has no index data");
    }

    const vk::DeviceSize bufferSize = sizeof(uint32_t) * indices.size();

    auto [stagingBuffer, stagingBufferMemory] =
        CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
                     vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    void* data = stagingBufferMemory.mapMemory(0, bufferSize);
    std::memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
    stagingBufferMemory.unmapMemory();

    std::tie(_indexBuffer, _indexBufferMemory) =
        CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
                     vk::MemoryPropertyFlagBits::eDeviceLocal);

    CopyBuffer(stagingBuffer, _indexBuffer, bufferSize);
}
void RendererVulkan::CreateUniformBuffers()
{
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vk::DeviceSize bufferSize = sizeof(UniformBufferObject);
        auto [buffer, bufferMem] =
            CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eUniformBuffer,
                         vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
        _uniformBuffers.emplace_back(std::move(buffer));
        _uniformBuffersMemory.emplace_back(std::move(bufferMem));
        _uniformBuffersMapped.emplace_back(_uniformBuffersMemory.back().mapMemory(0, bufferSize));
    }
}

void RendererVulkan::CreateDescriptorPool()
{
    std::array<vk::DescriptorPoolSize, 2> poolSize{
        {{.type = vk::DescriptorType::eUniformBuffer, .descriptorCount = MAX_FRAMES_IN_FLIGHT},
         {.type = vk::DescriptorType::eCombinedImageSampler, .descriptorCount = MAX_FRAMES_IN_FLIGHT}}};

    vk::DescriptorPoolCreateInfo poolInfo{.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
                                          .maxSets = MAX_FRAMES_IN_FLIGHT,
                                          .poolSizeCount = static_cast<uint32_t>(poolSize.size()),
                                          .pPoolSizes = poolSize.data()};

    _descriptorPool = vk::raii::DescriptorPool(_device, poolInfo);
}
void RendererVulkan::CreateDescriptorSets()
{
    std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, *_descriptorSetLayout);
    vk::DescriptorSetAllocateInfo allocInfo{.descriptorPool = _descriptorPool,
                                            .descriptorSetCount = static_cast<uint32_t>(layouts.size()),
                                            .pSetLayouts = layouts.data()};
    _descriptorSets = _device.allocateDescriptorSets(allocInfo);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vk::DescriptorBufferInfo bufferInfo{
            .buffer = _uniformBuffers[i], .offset = 0, .range = sizeof(UniformBufferObject)};
        const vk::DescriptorImageInfo imageInfo{.sampler = *_textureSampler,
                                                .imageView = *_textureImageView,
                                                .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal};
        const std::array<vk::WriteDescriptorSet, 2> writes{
            {{.dstSet = *_descriptorSets[i],
              .dstBinding = 0,
              .descriptorCount = 1,
              .descriptorType = vk::DescriptorType::eUniformBuffer,
              .pBufferInfo = &bufferInfo},
             {.dstSet = *_descriptorSets[i],
              .dstBinding = 1,
              .descriptorCount = 1,
              .descriptorType = vk::DescriptorType::eCombinedImageSampler,
              .pImageInfo = &imageInfo}}};
        _device.updateDescriptorSets(writes, {});
    }
}

std::pair<vk::raii::Buffer, vk::raii::DeviceMemory>
RendererVulkan::CreateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties)
{
    vk::BufferCreateInfo bufferInfo{.size = size, .usage = usage, .sharingMode = vk::SharingMode::eExclusive};
    vk::raii::Buffer buffer = vk::raii::Buffer(_device, bufferInfo);
    vk::MemoryRequirements memRequirements = buffer.getMemoryRequirements();
    vk::MemoryAllocateInfo allocInfo{.allocationSize = memRequirements.size,
                                     .memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties)};
    vk::raii::DeviceMemory bufferMemory = vk::raii::DeviceMemory(_device, allocInfo);
    buffer.bindMemory(*bufferMemory, 0);
    return {std::move(buffer), std::move(bufferMemory)};
}

std::pair<vk::raii::Image, vk::raii::DeviceMemory>
RendererVulkan::CreateImage(uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling,
                            vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties)
{
    vk::ImageCreateInfo imageInfo{.imageType = vk::ImageType::e2D,
                                  .format = format,
                                  .extent = {width, height, 1},
                                  .mipLevels = 1,
                                  .arrayLayers = 1,
                                  .samples = vk::SampleCountFlagBits::e1,
                                  .tiling = tiling,
                                  .usage = usage,
                                  .sharingMode = vk::SharingMode::eExclusive};

    vk::raii::Image image = vk::raii::Image(_device, imageInfo);

    vk::MemoryRequirements memRequirements = image.getMemoryRequirements();
    vk::MemoryAllocateInfo allocInfo{.allocationSize = memRequirements.size,
                                     .memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties)};
    vk::raii::DeviceMemory imageMemory = vk::raii::DeviceMemory(_device, allocInfo);
    image.bindMemory(imageMemory, 0);

    return {std::move(image), std::move(imageMemory)};
}

void RendererVulkan::RecordCommandBuffer(uint32_t imageIndex)
{
    auto& commandBuffer = _commandBuffers[_frameIndex];
    commandBuffer.begin({});
    TransitionImageLayout(_swapChainImages[imageIndex], vk::ImageLayout::eUndefined,
                          vk::ImageLayout::eColorAttachmentOptimal,
                          {}, // srcAccessMask (no need to wait for previous operations)
                          vk::AccessFlagBits2::eColorAttachmentWrite,         // dstAccessMask
                          vk::PipelineStageFlagBits2::eColorAttachmentOutput, // srcStage
                          vk::PipelineStageFlagBits2::eColorAttachmentOutput, // dstStage
                          vk::ImageAspectFlagBits::eColor);

    const auto depthStages =
        vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests;
    TransitionImageLayout(*_depthImage, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthAttachmentOptimal,
                          vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
                          vk::AccessFlagBits2::eDepthStencilAttachmentRead |
                              vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
                          depthStages, depthStages, vk::ImageAspectFlagBits::eDepth);

    vk::ClearValue clearColor{.color = {.float32 = {{0.0f, 0.0f, 0.0f, 1.0f}}}};
    vk::ClearValue clearDepth{.depthStencil = {.depth = 1.0f, .stencil = 0}};

    vk::RenderingAttachmentInfo attachmentInfo = {.imageView = _swapChainImageViews[imageIndex],
                                                  .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
                                                  .loadOp = vk::AttachmentLoadOp::eClear,
                                                  .storeOp = vk::AttachmentStoreOp::eStore,
                                                  .clearValue = clearColor};

    vk::RenderingAttachmentInfo depthAttachmentInfo = {.imageView = _depthImageView,
                                                       .imageLayout = vk::ImageLayout::eDepthAttachmentOptimal,
                                                       .loadOp = vk::AttachmentLoadOp::eClear,
                                                       .storeOp = vk::AttachmentStoreOp::eDontCare,
                                                       .clearValue = clearDepth};

    vk::RenderingInfo renderingInfo = {.renderArea = {.offset = {0, 0}, .extent = _swapChainExtent},
                                       .layerCount = 1,
                                       .colorAttachmentCount = 1,
                                       .pColorAttachments = &attachmentInfo,
                                       .pDepthAttachment = &depthAttachmentInfo};

    commandBuffer.beginRendering(renderingInfo);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *_graphicsPipeline);
    commandBuffer.setViewport(0, vk::Viewport(0.0f, static_cast<float>(_swapChainExtent.height),
                                              static_cast<float>(_swapChainExtent.width),
                                              -static_cast<float>(_swapChainExtent.height), 0.0f, 1.0f));
    commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), _swapChainExtent));
    commandBuffer.bindVertexBuffers(0, *_vertexBuffer, {0});
    commandBuffer.bindIndexBuffer(*_indexBuffer, 0, vk::IndexType::eUint32);
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, _pipelineLayout, 0,
                                     *_descriptorSets[_frameIndex], nullptr);
    commandBuffer.drawIndexed(static_cast<uint32_t>(_mesh->_verticesIndices.size()), 1, 0, 0, 0);

    commandBuffer.endRendering();

    // After rendering, transition the swapchain image to vk::ImageLayout::ePresentSrcKHR.
    TransitionImageLayout(_swapChainImages[imageIndex], vk::ImageLayout::eColorAttachmentOptimal,
                          vk::ImageLayout::ePresentSrcKHR,
                          vk::AccessFlagBits2::eColorAttachmentWrite,         // srcAccessMask
                          {},                                                 // dstAccessMask
                          vk::PipelineStageFlagBits2::eColorAttachmentOutput, // srcStage
                          vk::PipelineStageFlagBits2::eBottomOfPipe,          // dstStage
                          vk::ImageAspectFlagBits::eColor);
    commandBuffer.end();
}

void RendererVulkan::TransitionImageLayout(vk::Image image, vk::ImageLayout old_layout, vk::ImageLayout new_layout,
                                           vk::AccessFlags2 src_access_mask, vk::AccessFlags2 dst_access_mask,
                                           vk::PipelineStageFlags2 src_stage_mask,
                                           vk::PipelineStageFlags2 dst_stage_mask, vk::ImageAspectFlags aspectFlags)
{
    vk::ImageMemoryBarrier2 barrier = {
        .srcStageMask = src_stage_mask,
        .srcAccessMask = src_access_mask,
        .dstStageMask = dst_stage_mask,
        .dstAccessMask = dst_access_mask,
        .oldLayout = old_layout,
        .newLayout = new_layout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange = {
            .aspectMask = aspectFlags, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1}};
    vk::DependencyInfo dependency_info = {
        .dependencyFlags = {}, .imageMemoryBarrierCount = 1, .pImageMemoryBarriers = &barrier};
    _commandBuffers[_frameIndex].pipelineBarrier2(dependency_info);
}
void RendererVulkan::TransitionImageLayout(vk::raii::CommandBuffer& commandBuffer, const vk::raii::Image& image,
                                           vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
{
    vk::ImageMemoryBarrier barrier{
        .oldLayout = oldLayout,
        .newLayout = newLayout,
        .srcQueueFamilyIndex = vk::QueueFamilyIgnored,
        .dstQueueFamilyIndex = vk::QueueFamilyIgnored,
        .image = image,
        .subresourceRange = {.aspectMask = vk::ImageAspectFlagBits::eColor, .levelCount = 1, .layerCount = 1}};
    vk::PipelineStageFlags sourceStage;
    vk::PipelineStageFlags destinationStage;
    if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal)
    {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
        sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
        destinationStage = vk::PipelineStageFlagBits::eTransfer;
    }
    else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
    {
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
        sourceStage = vk::PipelineStageFlagBits::eTransfer;
        destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
    }
    else
    {
        throw std::invalid_argument("Unsupported texture image layout transition");
    }

    commandBuffer.pipelineBarrier(sourceStage, destinationStage, {}, {}, {}, barrier);
}

void RendererVulkan::RecreateSwapChain()
{
    int width = 0;
    int height = 0;
    if (!SDL_GetWindowSizeInPixels(_window.GetSDLWindow(), &width, &height))
    {
        throw std::runtime_error(SDL_GetError());
    }
    if (width == 0 || height == 0)
    {
        return;
    }

    _device.waitIdle();
    CleanupSwapChain();
    CreateSwapChain();
    CreateImageViews();
    CreateDepthResources();
    _renderFinishedSemaphores.clear();
    for (size_t i = 0; i < _swapChainImages.size(); ++i)
    {
        _renderFinishedSemaphores.emplace_back(_device, vk::SemaphoreCreateInfo{});
    }
    _framebufferResized = false;
}

void RendererVulkan::CleanupSwapChain()
{
    _depthImageView.clear();
    _depthImage.clear();
    _depthImageMemory.clear();
    _swapChainImageViews.clear();
    _swapChain = nullptr;
}

uint32_t RendererVulkan::FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
{
    vk::PhysicalDeviceMemoryProperties memProperties = _physicalDevice.getMemoryProperties();

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw std::runtime_error("Failed to find suitable memory type !");
}
vk::Format RendererVulkan::FindSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling,
                                               vk::FormatFeatureFlags features)
{
    for (const auto format : candidates)
    {
        vk::FormatProperties props = _physicalDevice.getFormatProperties(format);
        if (((tiling == vk::ImageTiling::eLinear) && ((props.linearTilingFeatures & features) == features)) ||
            ((tiling == vk::ImageTiling::eOptimal) && ((props.optimalTilingFeatures & features) == features)))
        {
            return format;
        }
    }

    throw std::runtime_error("Failed to find supported format !");
}

vk::Format RendererVulkan::FindDepthFormat()
{
    return FindSupportedFormat({vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
                               vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}

void RendererVulkan::CopyBuffer(vk::raii::Buffer& srcBuffer, vk::raii::Buffer& dstBuffer, vk::DeviceSize size)
{
    vk::raii::CommandBuffer commandCopyBuffer = BeginSingleTimeCommands();
    commandCopyBuffer.copyBuffer(*srcBuffer, *dstBuffer, vk::BufferCopy{.size = size});
    EndSingleTimeCommands(std::move(commandCopyBuffer));
}
void RendererVulkan::CopyBufferToImage(vk::raii::CommandBuffer& commandBuffer, const vk::raii::Buffer& buffer,
                                       vk::raii::Image& image, uint32_t width, uint32_t height)
{
    vk::BufferImageCopy region{.bufferOffset = 0,
                               .bufferRowLength = 0,
                               .bufferImageHeight = 0,
                               .imageSubresource = {.aspectMask = vk::ImageAspectFlagBits::eColor,
                                                    .mipLevel = 0,
                                                    .baseArrayLayer = 0,
                                                    .layerCount = 1},
                               .imageOffset = {0, 0, 0},
                               .imageExtent = {width, height, 1}};
    commandBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, region);
}

void RendererVulkan::UpdateUniformBuffer(uint32_t currentImage)
{
    UniformBufferObject ubo{};
    // Matrix4 multiplication uses row indexing over column-major storage.
    ubo.model = Matrix4::translation(-_mesh->_pivot) * _accumulatedRotationMatrix;
    ubo.view = _viewMatrix;
    // Right-handed projection with Vulkan's [0, 1] depth range.
    // Y is already corrected by the negative viewport height.
    const float nearPlane = 0.1f;
    const float farPlane = 1000.0f;
    const float scale = 1.0f / std::tan(45.0f * static_cast<float>(M_PI) / 360.0f);
    ubo.proj = Matrix4(0.0f);
    ubo.proj._m[0][0] =
        scale * static_cast<float>(_swapChainExtent.height) / static_cast<float>(_swapChainExtent.width);
    ubo.proj._m[1][1] = scale;
    ubo.proj._m[2][2] = farPlane / (nearPlane - farPlane);
    ubo.proj._m[2][3] = -1.0f;
    ubo.proj._m[3][2] = (farPlane * nearPlane) / (nearPlane - farPlane);
    memcpy(_uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}
vk::raii::CommandBuffer RendererVulkan::BeginSingleTimeCommands()
{
    vk::CommandBufferAllocateInfo allocInfo{
        .commandPool = _commandPool, .level = vk::CommandBufferLevel::ePrimary, .commandBufferCount = 1};
    vk::raii::CommandBuffer commandBuffer = std::move(vk::raii::CommandBuffers(_device, allocInfo).front());

    vk::CommandBufferBeginInfo beginInfo{.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
    commandBuffer.begin(beginInfo);

    return commandBuffer;
}
void RendererVulkan::EndSingleTimeCommands(vk::raii::CommandBuffer&& commandBuffer)
{
    commandBuffer.end();

    vk::SubmitInfo submitInfo{.commandBufferCount = 1, .pCommandBuffers = &*commandBuffer};
    _queue.submit(submitInfo, nullptr);
    _queue.waitIdle();
}

RendererVulkan::~RendererVulkan()
{
    _device.waitIdle();
}

Matrix4 RendererVulkan::GetFinalMatrix(RotationAxis activeAxis, const Matrix4& accumulatedRotationMatrix)
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

void RendererVulkan::Start()
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
    CreateVertexBuffer();
    CreateIndexBuffer();
    CreateDepthResources();
    CreateTextureImage();
    CreateTextureImageView();
    CreateTextureSampler();
    CreateDescriptorSets();
}

void RendererVulkan::Update(float deltaTime, Camera& camera)
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

void RendererVulkan::Render()
{
    int width = 0;
    int height = 0;
    if (!SDL_GetWindowSizeInPixels(_window.GetSDLWindow(), &width, &height))
    {
        throw std::runtime_error(SDL_GetError());
    }
    if (width == 0 || height == 0)
    {
        return;
    }

    // No image has been acquired yet, so resize can be handled safely here.
    if (_framebufferResized)
    {
        RecreateSwapChain();
    }

    auto& fence = _inFlightFences[_frameIndex];
    auto& imageAvailable = _presentCompleteSemaphores[_frameIndex];
    auto& commandBuffer = _commandBuffers[_frameIndex];

    if (_device.waitForFences(*fence, vk::True, UINT64_MAX) != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to wait for frame fence");
    }

    auto [result, imageIndex] = _swapChain.acquireNextImage(UINT64_MAX, *imageAvailable, nullptr);
    if (result == vk::Result::eErrorOutOfDateKHR)
    {
        RecreateSwapChain();
        return;
    }
    if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
    {
        assert(result == vk::Result::eTimeout || result == vk::Result::eNotReady);
        throw std::runtime_error("Failed to acquire swapchain image");
    }

    auto& renderFinished = _renderFinishedSemaphores[imageIndex];
    commandBuffer.reset();
    RecordCommandBuffer(imageIndex);

    UpdateUniformBuffer(_frameIndex);

    const vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    const vk::SubmitInfo submitInfo{.waitSemaphoreCount = 1,
                                    .pWaitSemaphores = &*imageAvailable,
                                    .pWaitDstStageMask = &waitStage,
                                    .commandBufferCount = 1,
                                    .pCommandBuffers = &*commandBuffer,
                                    .signalSemaphoreCount = 1,
                                    .pSignalSemaphores = &*renderFinished};
    _device.resetFences(*fence);
    _queue.submit(submitInfo, *fence);

    const vk::PresentInfoKHR presentInfo{.waitSemaphoreCount = 1,
                                         .pWaitSemaphores = &*renderFinished,
                                         .swapchainCount = 1,
                                         .pSwapchains = &*_swapChain,
                                         .pImageIndices = &imageIndex};
    result = _queue.presentKHR(presentInfo);
    if (result == vk::Result::eSuboptimalKHR || result == vk::Result::eErrorOutOfDateKHR || _framebufferResized)
    {
        RecreateSwapChain();
    }
    else if (result != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to present swapchain image");
    }
    _frameIndex = (_frameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
}

void RendererVulkan::SwapBuffers()
{
}

void RendererVulkan::SetPolygonMode(RenderMode mode)
{
    (void)mode;
}
