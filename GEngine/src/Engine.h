//
// Created by gaetz on 08/06/2021.
//

#ifndef ENGINE_H
#define ENGINE_H

#include <vector>
#include <unordered_map>
#include <array>
#include <vk_mem_alloc.h>

#include "Defines.h"
#include "vk/RenderObject.h"
#include "platforms/PlatformWin.h"
#include "input/InputSystem.h"
#include "vk/DeletionQueue.h"

using std::vector;
using std::unordered_map;
using std::array;

using engine::input::InputSystem;
using engine::vk::DeletionQueue;
using engine::vk::Mesh;
using engine::vk::RenderObject;
using engine::platforms::Platform;
using engine::platforms::PlatformWin;

// Buffer this number of frames when rendering
constexpr unsigned int FRAME_OVERLAP = 2;
constexpr unsigned int MAX_OBJECTS = 10000;

namespace engine {

class Engine {
public:

    GAPI Engine();
    GAPI ~Engine();

    bool isInitialized { false };
    int frameNumber { 0 };
    VkExtent2D windowExtent { 1280, 720 };
    bool isRunning { false };

    // Platform
    #ifdef GPLATFORM_WINDOWS
    Platform* platform = new PlatformWin();
    #else
    Platform platform* = nullptr;
    // No implementation, won't compile
    #endif

    // Instance and devices

    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkPhysicalDevice chosenGPU;
    VkDevice device;
    VkPhysicalDeviceProperties gpuProperties;

    // Swapchain

    VkSurfaceKHR surface;
    VkSwapchainKHR swapchain;
    VkFormat swapchainImageFormat;
    vector<VkImage> swapchainImages;
    vector<VkImageView> swapchainImageViews;

    // Queues and commands

    VkQueue graphicsQueue;
    uint32_t graphicsQueueFamily;

    // Render pass and synchronisation

    VkRenderPass renderPass;
    vector<VkFramebuffer> framebuffers;
    array<vk::FrameData, FRAME_OVERLAP> frames;

    // Pipeline

    VkPipeline meshPipeline;
    VkPipelineLayout texturedMeshPipelineLayout;
    DeletionQueue mainDeletionQueue;

    // Allocator

    VmaAllocator allocator;

    // Meshes

    vector<RenderObject> renderables;
    unordered_map<string, vk::Material> materials;
    unordered_map<string, Mesh> meshes;

    // Depth

    VkImageView depthImageView;
    vk::AllocatedImage depthImage;
    VkFormat depthFormat;

    // Descriptor sets

    VkDescriptorSetLayout globalSetLayout;
    VkDescriptorPool descriptorPool;
    VkDescriptorSetLayout objectSetLayout;
    VkDescriptorSetLayout singleTextureSetLayout;

    // Scene data

    vk::GPUSceneData sceneParams;
    AllocatedBuffer sceneParamsBuffer;

    // Transfer and textures

    vk::UploadContext uploadContext;
    unordered_map<string, vk::Texture> textures;


    // Initializes everything in the engine
    GAPI void init();

    // Shuts down the engine
    GAPI void cleanup();

    // Process engine inputs
    GAPI const input::InputState processInputs();

    // Draw loop
    GAPI void draw();

    // Run main loop
    //void run();

    // Get the frame we are rendering right now
    vk::FrameData& getCurrentFrame() { return frames[frameNumber % FRAME_OVERLAP]; }

    // Create a vulkan buffer
    AllocatedBuffer createBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);

private:
    InputSystem inputSystem { windowExtent.width, windowExtent.height };

    // Setup vulkan

    void initVulkan();

    void initSwapchain();

    void initCommands();

    void initDefaultRenderpass();

    void initFramebuffers();

    void initSyncStructures();

    void initDescriptors();

    void initPipelines();

    void initScene();

    // Shaders and buffers

    bool loadShaderModule(const char* path, VkShaderModule* outShaderModule);
    size_t padUniformBufferSize(size_t originalSize) const;

    // Meshes

    void loadMeshes();
    void uploadMesh(Mesh& mesh);
    vk::Material* createMaterial(VkPipeline pipelineP, VkPipelineLayout pipelineLayoutP, const string& name);
    vk::Material* getMaterial(const string& name);
    Mesh* getMesh(const string& name);
    void loadImages();
    bool loadImageFromFile(const string& path, vk::AllocatedImage& outImage);

    // Draw & commands

    void drawObjects(VkCommandBuffer cmd, RenderObject* first, size_t count);
    void immediateSubmit(std::function<void(VkCommandBuffer cmd)>&& submittedFunc);

    // Clean

    void cleanupVulkan();
};

}
#endif //ENGINE_H
