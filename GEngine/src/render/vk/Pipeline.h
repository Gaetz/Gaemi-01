//
// Created by gaetz on 26/10/2021.
//

#ifndef RENDER_VK_PIPELINE_H
#define RENDER_VK_PIPELINE_H

#include <vulkan/vulkan_core.h>
#include <vector>
#include "../../Defines.h"

using std::vector;

namespace engine::render::vk {
    class Context;
    class Renderpass;
    class CommandBuffer;

    class PipelineBuilder {
    public:
        vector<VkPipelineShaderStageCreateInfo> shaderStages;
        VkPipelineVertexInputStateCreateInfo vertexInputInfo;
        VkPipelineInputAssemblyStateCreateInfo inputAssembly;
        VkViewport viewport;
        VkRect2D scissor;
        VkPipelineRasterizationStateCreateInfo rasterizer;
        VkPipelineColorBlendAttachmentState colorBlendAttachment;
        VkPipelineMultisampleStateCreateInfo multisampling;
        VkPipelineLayout pipelineLayout;
        VkPipelineDepthStencilStateCreateInfo depthStencil;

        VkPipeline buildPipeline(const Context& context, const Renderpass& pass, bool isWireframe);
    };

    class Pipeline {
    public:
        explicit Pipeline(Context& context, const Renderpass& renderpass);
        VkPipeline handle;
        VkPipelineLayout pipelineLayout;

        void init(PipelineBuilder& builder, bool isWireframe);
        void destroy();
        void bind(const CommandBuffer& commandBuffer, VkPipelineBindPoint bindPoint) const;

    private:
        Context& context;
        const Renderpass& renderpass;
    };
}


#endif //RENDER_VK_PIPELINE_H
