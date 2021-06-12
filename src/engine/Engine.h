//
// Created by gaetz on 08/06/2021.
//

#ifndef ENGINE_H
#define ENGINE_H

#include <vector>
#include "vk/Types.h"
#include "Window.h"
#include "Game.h"
#include "../engine/input/InputSystem.h"

using std::vector;
using engine::input::InputSystem;
using game::Game;

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

    // Clean
    void cleanupVulkan();
};

}
#endif //ENGINE_H
