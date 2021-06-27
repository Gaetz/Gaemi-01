//
// Created by gaetz on 08/06/2021.
//

#ifndef ENGINE_H
#define ENGINE_H

#include <vector>
#include <unordered_map>
#include <array>
#include <DeletionQueue.h>
#include <vk_mem_alloc.h>

#include "vk/RenderObject.h"
#include "Window.h"
#include "Game.h"
#include "input/InputSystem.h"

using std::vector;
using std::unordered_map;
using std::array;

using engine::input::InputSystem;
using game::Game;
using engine::vk::DeletionQueue;
using engine::vk::Mesh;
using engine::vk::RenderObject;

// Buffer this number of frames when rendering
constexpr unsigned int FRAME_OVERLAP = 2;

namespace engine {

class Engine {
public:

    Engine();

    bool isInitialized;
    int frameNumber;
    VkExtent2D windowExtent;
    Window window;

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
    VkPipelineLayout meshPipelineLayout;
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

    // Scene data

    vk::GPUSceneData sceneParams;
    AllocatedBuffer sceneParamsBuffer;

    // Transfer and textures

    vk::UploadContext uploadContext;
    unordered_map<string, vk::Texture> textures;


    // Initializes everything in the engine
    void init();

    // Shuts down the engine
    void cleanup();

    // Process engine inputs
    void processInputs();

    // Draw loop
    void draw();

    // Run main loop
    void run();

    // Get the frame we are rendering right now
    vk::FrameData& getCurrentFrame() { return frames[frameNumber % FRAME_OVERLAP]; }

    // Create a vulkan buffer
    AllocatedBuffer createBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);

private:
    Game game;
    InputSystem inputSystem;

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
