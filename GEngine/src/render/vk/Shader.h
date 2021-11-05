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
        explicit Shader(Context& context, Renderpass& renderpass);

        void init(const string& shaderName);

        void destroy();

        void use();

        vector<VkPipelineShaderStageCreateInfo> getStagesCreateInfo();

        // Shader name
        string name;


    private:
        array<ShaderStage, SHADER_STAGE_COUNT> stages;
        Context& context;
        Renderpass& renderpass;
        Pipeline pipeline { context, renderpass };

        bool load(const string& shaderName, const string& typeStrings, VkShaderStageFlagBits shaderStageFlagBit, ShaderStage& shaderStage);

    };

}
#endif //RENDER_VK_SHADER_H
