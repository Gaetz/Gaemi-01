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

using std::string;
using std::array;
using std::vector;

constexpr int SHADER_STAGE_COUNT { 2 };

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
        void init(const Context& context, const array<VkDescriptorSetLayout, 3>& setLayouts,
                  const Renderpass& renderpass, const string& shaderName);

        void destroy();

        void use(const CommandBuffer& cmd, VkPipelineBindPoint bindPoint) const;

        vector<VkPipelineShaderStageCreateInfo> getStagesCreateInfo();

        // Shader name
        string name;

        // Shader associated pipeline
        Pipeline pipeline;

    private:
        array<ShaderStage, SHADER_STAGE_COUNT> stages;
        VkDevice contextDevice;

        bool load(const Context& context, const string& shaderName, const string& typeStrings, VkShaderStageFlagBits shaderStageFlagBit, ShaderStage& shaderStage);
        void initPipelineLayout(const Context& context, const array<VkDescriptorSetLayout, 3>& setLayouts);
        void initPipeline(const Context& context, const Renderpass& renderpass);
    };

}
#endif //RENDER_VK_SHADER_H
