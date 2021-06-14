//
// Created by gaetz on 12/06/2021.
//

#ifndef VK_PIPELINEBUILDER_H
#define VK_PIPELINEBUILDER_H

#include "vulkan/vulkan.h"

#include <vector>

using std::vector;

namespace engine::vk {

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

        VkPipeline buildPipeline(VkDevice device, VkRenderPass pass);

    };

}
#endif //VK_PIPELINEBUILDER_H
