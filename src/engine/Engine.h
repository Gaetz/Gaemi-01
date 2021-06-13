//
// Created by gaetz on 08/06/2021.
//

#ifndef ENGINE_H
#define ENGINE_H

#include <vector>
#include <DeletionQueue.h>
#include <vk_mem_alloc.h>
#include <Mesh.h>

#include "vk/Types.h"
#include "Window.h"
#include "Game.h"
#include "input/InputSystem.h"

using std::vector;
using engine::input::InputSystem;
using game::Game;
using engine::vk::DeletionQueue;
using engine::vk::Mesh;

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

    VkPipeline trianglePipeline;
    VkPipeline redTrianglePipeline;
    VkPipelineLayout trianglePipelineLayout;
    DeletionQueue mainDeletionQueue;

    // Allocator

    VmaAllocator allocator;

    // Meshes

    VkPipeline meshPipeline;
    Mesh triangleMesh;
    VkPipelineLayout meshPipelineLayout;

    // Getters and setters

    InputSystem& getInputSystem() { return inputSystem; }

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

    // Shaders

    bool loadShaderModule(const char* path, VkShaderModule* outShaderModule);

    // Meshes

    void loadMeshes();
    void uploadMesh(Mesh& mesh);

    // Clean

    void cleanupVulkan();
};

}
#endif //ENGINE_H
