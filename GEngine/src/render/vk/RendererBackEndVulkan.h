//
// Created by gaetz on 08/10/2021.
//

#ifndef RENDERER_VK_RENDERERBACKENDVULKAN_H
#define RENDERER_VK_RENDERERBACKENDVULKAN_H

#include <vk_mem_alloc.h>

#include <string>

using std::string;

#include <vector>

using std::vector;

#include <unordered_map>

using std::unordered_map;

#include "../RendererBackEnd.h"
#include "RenderObject.h"
#include "DeletionQueue.h"
#include "Context.h"

namespace engine::render::vk {

    class RendererBackEndVulkan : public RendererBackEnd {
    public:
        RendererBackEndVulkan() = default;

        ~RendererBackEndVulkan() override = default;

        bool init(const string &appName, u16 width, u16 height) override;

        void close() override;

        bool beginFrame(u32 dt) override;

        bool endFrame(u32 dt) override;

        void resize() override;

        // Get the frame we are rendering right now
        render::vk::FrameData &getCurrentFrame() { return frames[frameNumber % FRAME_OVERLAP]; }

        // Create a vulkan buffer
        AllocatedBuffer createBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);


    private:
        Context context;

        // Swapchain

        uint32_t swapchainImageIndex { 0 };
        VkSwapchainKHR swapchain;
        VkFormat swapchainImageFormat;
        vector<VkImage> swapchainImages;
        vector<VkImageView> swapchainImageViews;

        // Render pass and synchronisation

        VkRenderPass renderPass;
        vector<VkFramebuffer> framebuffers;
        array<FrameData, FRAME_OVERLAP> frames;

        // Pipeline

        VkPipeline meshPipeline;
        VkPipelineLayout texturedMeshPipelineLayout;

        // Depth

        VkImageView depthImageView;
        render::vk::AllocatedImage depthImage;
        VkFormat depthFormat;

        // Descriptor sets

        VkDescriptorSetLayout globalSetLayout;
        VkDescriptorPool descriptorPool;
        VkDescriptorSetLayout objectSetLayout;
        VkDescriptorSetLayout singleTextureSetLayout;

        // Meshes

        vector<RenderObject> renderables;
        unordered_map<string, render::vk::Material> materials;
        unordered_map<string, Mesh> meshes;

        // Scene data

        render::vk::GPUSceneData sceneParams;
        AllocatedBuffer sceneParamsBuffer;

        // Transfer and textures

        render::vk::UploadContext uploadContext;
        unordered_map<string, render::vk::Texture> textures;

        void initSwapchain();

        void initCommands();

        void initDefaultRenderpass();

        void initFramebuffers();

        void initSyncStructures();

        void initDescriptors();

        void initPipelines();

        void initScene();

        // Shaders and buffers

        bool loadShaderModule(const char *path, VkShaderModule *outShaderModule);

        size_t padUniformBufferSize(size_t originalSize) const;

        // Meshes

        void loadMeshes();

        void uploadMesh(Mesh &mesh);

        render::vk::Material *
        createMaterial(VkPipeline pipelineP, VkPipelineLayout pipelineLayoutP, const string &name);

        render::vk::Material *getMaterial(const string &name);

        Mesh *getMesh(const string &name);

        void loadImages();

        bool loadImageFromFile(const string &path, render::vk::AllocatedImage &outImage);

        // Draw & commands

        void drawObjects(VkCommandBuffer cmd, RenderObject *first, size_t count);

        void immediateSubmit(std::function<void(VkCommandBuffer cmd)> &&submittedFunc);

        // Clean

        void cleanupVulkan();

    };

}

#endif //RENDERER_VK_RENDERERBACKENDVULKAN_H
