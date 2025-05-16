//
// Created by gaetz on 08/10/2021.
//

#ifndef RENDER_VK_RENDERERBACKENDVULKAN_H
#define RENDER_VK_RENDERERBACKENDVULKAN_H

#include <vma/vk_mem_alloc.h>
#include <vector>
#include <unordered_map>
#include "../RendererBackEnd.h"
#include "GameObject.h"
#include "DeletionQueue.h"
#include "Swapchain.h"
#include "Context.h"
#include "Renderpass.h"
#include "Framebuffer.h"
#include "Shader.h"
#include "Texture.h"

using std::vector;
using std::unordered_map;

namespace engine::render::vk {

    class RendererBackEndVulkan : public RendererBackEnd {
    public:
        RendererBackEndVulkan() = default;

        ~RendererBackEndVulkan() override = default;

        bool init(const string &appName, u16 width, u16 height) override;

        void close() override;

        bool beginFrame(u32 dt) override;

        void draw() override;

        bool endFrame(u32 dt) override;

        void resize() override;

        // Get the frame we are rendering right now
        FrameData &getCurrentFrame() { return frames[frameNumber % FRAME_OVERLAP]; }

        void updateGlobalState(Mat4 projection, Mat4 view) override;

        Texture loadTexture(const string& path) override;
        void uploadMesh(Mesh &mesh) override;
        void addToScene(vk::GameObject& gameObject) override;
        void waitIdle() override;

    private:
        Context context;
        Swapchain swapchain { context };

        // Render pass and synchronisation
        Renderpass renderpass { context,
                                { 0, 0, context.windowExtent.width, context.windowExtent.height },
                                { 0, 0, 0.2, 0 },
                                1.0f,
                                0 };
        vector<Framebuffer> framebuffers {};
        array<FrameData, FRAME_OVERLAP> frames;

        // Pipeline
        //Shader defaultShader { context, renderpass };
        //Pipeline meshPipeline { context, renderpass };
        //VkPipeline meshPipeline;
        //VkPipelineLayout texturedMeshPipelineLayout;

        // Descriptor sets

        VkDescriptorSetLayout globalSetLayout;
        VkDescriptorPool descriptorPool;
        VkDescriptorSetLayout objectSetLayout;
        VkDescriptorSetLayout singleTextureSetLayout;

        // Meshes

        vector<GameObject> renderables;
        unordered_map<string, render::vk::Material> materials;
        //unordered_map<string, Mesh> meshes;

        // Scene data

        render::vk::GPUSceneData sceneParams;
        AllocatedBuffer sceneParamsBuffer;

        // Transfer and textures

        render::vk::UploadContext uploadContext;

        bool loadTextureFromFile(const string &path, render::vk::AllocatedImage &outImage);

        // Initialization

        void initCommands();

        void regenerateFramebuffers();

        void initSyncStructures();

        void initDescriptors();

        void initImgui();

        void loadDefaultAssets();

        void createMaterial(const string& name, const string& shaderName, const string& textureName) override;

        // Buffers
        // Create a vulkan buffer
        AllocatedBuffer createBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);

        size_t padUniformBufferSize(size_t originalSize) const;

        // Draw & commands

        void drawObjects(CommandBuffer& commandBuffer, GameObject *first, size_t count);

        void immediateSubmit(std::function<void(VkCommandBuffer cmd)> &&submittedFunc);

        // Clean

        void cleanupVulkan();

    };

}

#endif //RENDER_VK_RENDERERBACKENDVULKAN_H
