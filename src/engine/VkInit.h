//
// Created by gaetz on 08/06/2021.
//

#ifndef VKINIT_H
#define VKINIT_H

#include "Types.h"

namespace vkinit {

    // Create a command pool for commands submitted to the graphics queue.
    VkCommandPoolCreateInfo commandPoolCreateInfo(uint32_t queueFamilyIndex,
                                                  VkCommandPoolCreateFlags flags = 0);

    // Allocate the default command buffer that we will use for rendering
    VkCommandBufferAllocateInfo commandBufferAllocateInfo(VkCommandPool pool,
                                                          uint32_t count = 1,
                                                          VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    // Holds information about a single shader stage for the pipeline
    VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfo(VkShaderStageFlagBits shaderStage,
                                                                  VkShaderModule shaderModule);

    // Contains the information for vertex buffers and vertex formats.
    VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo();

    // Contains the configuration for what kind of topology will be drawn.
    // This is where you set it to draw triangles, lines, points, or others like triangle-list.
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo(VkPrimitiveTopology topology);

    // Configuration for the fixed-function rasterization. In here is where we enable or disable
    // backface culling, and set line width or wireframe drawing.
    VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo(VkPolygonMode polygonMode);

    // Configure MSAA for this pipeline
    VkPipelineMultisampleStateCreateInfo multisamplingStateCreateInfo();

    // Controls how this pipeline blends into a given attachment.
    VkPipelineColorBlendAttachmentState colorBlendAttachmentState();

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo();
}



#endif //VKINIT_H
