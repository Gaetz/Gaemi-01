//
// Created by gaetz on 08/06/2021.
//

#ifndef ENGINE_H
#define ENGINE_H

#include <vector>
#include <unordered_map>
#include <DeletionQueue.h>
#include <vk_mem_alloc.h>

#include "vk/RenderObject.h"
#include "Window.h"
#include "Game.h"
#include "input/InputSystem.h"

using std::vector;
using std::unordered_map;

using engine::input::InputSystem;
using game::Game;
using engine::vk::DeletionQueue;
using engine::vk::Mesh;
using engine::vk::RenderObject;

constexpr int MAX_SHADERS = 4;

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

    // Swapchain

    VkSurfaceKHR surface;
    VkSwapchainKHR swapchain;
    VkFormat swapchainImageFormat;
    vector<VkImage> swapchainImages;
    vector<VkImageView> swapchainImageViews;

    // Queues and commands

    VkQueue graphicsQueue;
    uint32_t graphicsQueueFamily;
    VkCommandPool commandPool;
    VkCommandBuffer mainCommandBuffer;

    // Render pass and synchronisation

    VkRenderPass renderPass;
    vector<VkFramebuffer> framebuffers;
    VkSemaphore presentSemaphore, renderSemaphore;
    VkFence renderFence;

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

private:
    Game game;
    InputSystem inputSystem;
    int selectedShader;

    // Setup vulkan

    void initVulkan();

    void initSwapchain();

    void initCommands();

    void initDefaultRenderpass();

    void initFramebuffers();

    void initSyncStructures();

    void initPipelines();

    void initScene();

    // Shaders

    bool loadShaderModule(const char* path, VkShaderModule* outShaderModule);

    // Meshes

    void loadMeshes();
    void uploadMesh(Mesh& mesh);
    vk::Material* createMaterial(VkPipeline pipelineP, VkPipelineLayout pipelineLayoutP, const string& name);
    vk::Material* getMaterial(const string& name);
    Mesh* getMesh(const string& name);

    // Draw

    void drawObjects(VkCommandBuffer cmd, RenderObject* first, size_t count);

    // Clean

    void cleanupVulkan();
};

}
#endif //ENGINE_H
