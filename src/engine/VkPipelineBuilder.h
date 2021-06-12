//
// Created by gaetz on 12/06/2021.
//

#ifndef VKPIPELINEBUILDER_H
#define VKPIPELINEBUILDER_H

#include "vulkan/vulkan.h"

#include <vector>
using std::vector;

class VkPipelineBuilder {
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

    VkPipeline buildPipeline(VkDevice device, VkRenderPass pass);

};


#endif //VKPIPELINEBUILDER_H
