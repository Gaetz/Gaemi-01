//
// Created by gaetz on 26/10/2021.
//

#ifndef RENDER_VK_SHADER_H
#define RENDER_VK_SHADER_H

#include <string>
#include <array>
#include <vector>
#include <vulkan/vulkan_core.h>
#include "Pipeline.h"
#include "../RendererBackEnd.h"
#include "Buffer.h"

using std::string;
using std::array;
using std::vector;

constexpr i32 SHADER_STAGE_COUNT { 2 };

namespace engine::render::vk {

    class Context;

    class ShaderStage {
    public:
        VkShaderModule moduleHandle;
        VkShaderModuleCreateInfo moduleCreateInfo {};
        VkPipelineShaderStageCreateInfo stageCreateInfo {};
    };

    class Shader {
    public:
        // Shader name
        string name;

        // Shader associated pipeline
        Pipeline pipeline;

        // Descriptor sets

        VkDescriptorSetLayout globalSetLayout;
        VkDescriptorSetLayout objectSetLayout;
        VkDescriptorSetLayout singleTextureSetLayout;

        array<VkDescriptorSet, FRAME_OVERLAP> globalDescriptors;
        Buffer cameraBuffer;
        Buffer sceneBuffer;

        array<VkDescriptorSet, FRAME_OVERLAP> objectDescriptors;
        Buffer objectBuffer;

        void init(Context& context, array<VkDescriptorSetLayout, 3> setLayouts, const Renderpass& renderpass, VkDescriptorPool descriptorPool, const string& shaderName);

        void destroy();

        void use(const CommandBuffer& cmd, VkPipelineBindPoint bindPoint) const;

        void updateGlobalState(CommandBuffer& commandBuffer, Buffer& buffer, u64 dataSize, void* data,
                               VkDescriptorSet descriptorSet, u32 firstSet = 0, u32 descriptorSetCount = 1,
                               VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                               u32 dynamicOffsetCount = 0, u32 dynamicOffsets = 0);

        vector<VkPipelineShaderStageCreateInfo> getStagesCreateInfo();


        //void compileShader(const string& src, const string& dst);

    private:
        // Shader stages
        array<ShaderStage, SHADER_STAGE_COUNT> stages;

        // Save device for destruction
        VkDevice contextDevice;

        bool load(const Context& context, const string& shaderName, const string& typeStrings, VkShaderStageFlagBits shaderStageFlagBit, ShaderStage& shaderStage);
        void initPipelineLayout(const array<VkDescriptorSetLayout, 3>& setLayouts);
        void initPipeline(const Context& context, const Renderpass& renderpass);

        //bool compileShaderFile(const string& src);
        //void saveCompiledShader(const string& dst);
    };

}
#endif //RENDER_VK_SHADER_H
