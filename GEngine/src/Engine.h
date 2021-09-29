//
// Created by gaetz on 08/06/2021.
//

#ifndef ENGINE_H
#define ENGINE_H

#include <vector>
#include <unordered_map>
#include <array>
#include <vk_mem_alloc.h>
#include <functional>

#include "Defines.h"
#include "Game.h"
#include "vk/RenderObject.h"
#include "platforms/PlatformWin.h"
#include "input/InputSystem.h"
#include "vk/DeletionQueue.h"
#include "../../GTestBed/src/GameImpl.h"
#include "Memory.h"

using std::vector;
using std::unordered_map;
using std::array;
using std::unique_ptr;
using std::make_unique;

using engine::input::InputSystem;
using engine::vk::DeletionQueue;
using engine::vk::Mesh;
using engine::vk::RenderObject;
using engine::platforms::Platform;
using engine::platforms::PlatformWin;
using game::Game;
using engine::Memory;

// Buffer this number of frames when rendering
constexpr unsigned int FRAME_OVERLAP = 2;
constexpr unsigned int MAX_OBJECTS = 10000;

namespace engine {

struct EngineConfig {
    i16 startPositionX;
    i16 startPositionY;
    u16 startWidth;
    u16 startHeight;
    string name;
    bool fullscreenMode;
};

struct EngineState {
    bool isInitialized { false };
    bool isRunning { false };
    bool isPaused { false };
    u64 frameNumber { 0 };
    Game* game;
    Memory memoryManager;

    // Platform
    #ifdef GPLATFORM_WINDOWS
    Platform* platform = new PlatformWin();
    #else
    Platform* platform { nullptr };
    // No implementation, won't compile
    #endif
};

class Engine {
private:
    static EngineState state;
    EngineConfig config;

public:
    GAPI explicit Engine(const EngineConfig& configP);
    GAPI ~Engine() = default;

    GAPI static EngineState& getState();

    VkExtent2D windowExtent;
    InputSystem inputSystem;

    /*
    bool isInitialized { false };
    int frameNumber { 0 };
    bool isRunning { false };

    // Platform
    #ifdef GPLATFORM_WINDOWS
    unique_ptr<PlatformWin> platform = make_unique<PlatformWin>();
    #else
    Platform* platform = nullptr;
    // No implementation, won't compile
    #endif
    */


    // Initializes everything in the engine
    void init(Game& game, u64 sizeOfGameClass);

    // Run the engine
    void run();

    // Shuts down the engine
    void cleanup();

    // Process engine inputs
    input::InputState processInputs();

    // Update loop
    void update(u32 dt);

    // Draw loop
    void draw();

    // Get time since game started in milliseconds
    GAPI u64 getAbsoluteTime() const;

    // Get time since game started in seconds
    GAPI f64 getAbsoluteTimeSeconds() const;

    // Make the engine sleep for a time in milliseconds
    GAPI void sleep(u64 ms) const;

    // Get a formatted date
    GAPI std::array<char, 19> getDate();




    // VULKAN

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

    // Get the frame we are rendering right now
    vk::FrameData& getCurrentFrame() { return frames[state.frameNumber % FRAME_OVERLAP]; }

    // Create a vulkan buffer
    AllocatedBuffer createBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);

private:
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
