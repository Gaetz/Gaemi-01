//
// Created by gaetz on 08/06/2021.
//

#ifndef ENGINE_H
#define ENGINE_H

#include <vector>
using std::vector;

#include "Types.h"
#include "Window.h"
#include "Game.h"

class Engine
{
public:

    bool isInitialized{ false };
    int frameNumber {0};
    VkExtent2D windowExtent{ 1280 , 720 };
    Window window {"Gaemi-01"};

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
    VkPipelineLayout trianglePipelineLayout;

    // Initializes everything in the engine
    void init();

    // Shuts down the engine
    void cleanup();

    // Draw loop
    void draw();

    // Run main loop
    void run();

private:
    Game game;

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

    // Clean
    void cleanupVulkan();
};

#endif //ENGINE_H
