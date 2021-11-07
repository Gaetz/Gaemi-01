//
// Created by gaetz on 08/06/2021.
//

#ifndef RENDER_VK_INIT_H
#define RENDER_VK_INIT_H

#include "Types.h"

#include <string>
using std::string;

namespace engine {
class Engine;
}

namespace engine::render::vk {

    class Context;

    // Pad a buffer
    size_t padUniformBufferSize(Context& context, size_t originalSize);

    // Create a command pool for commands submitted to the graphics queue.
    VkCommandPoolCreateInfo commandPoolCreateInfo(uint32_t queueFamilyIndex,
                                                  VkCommandPoolCreateFlags flags = 0);

    // Allocate the default command buffer that we will use for rendering
    VkCommandBufferAllocateInfo commandBufferAllocateInfo(VkCommandPool pool,
                                                          uint32_t count = 1,
                                                          VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    // Holds information about a single shader stage for the pipeline
    VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfo(VkShaderStageFlagBits shaderStage,
                                                                  VkShaderModule shaderModule,
                                                                  const char* name);

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

    // Generate a layout for a pipeline
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo();

    // Generate image
    VkImageCreateInfo imageCreateInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent);

    // Generate image view
    VkImageViewCreateInfo imageViewCreateInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags);

    // Info about how to use depth-testing on a render pipeline
    VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo(bool depthTest, bool depthWrite, VkCompareOp compareOp);

    // Create semaphore info
    VkSemaphoreCreateInfo semaphoreCreateInfo();

    // Create fence info
    VkFenceCreateInfo fenceCreateInfo(VkFenceCreateFlagBits flags = static_cast<VkFenceCreateFlagBits>(0));

    // Create descriptor set layout binding
    VkDescriptorSetLayoutBinding descriptorSetLayoutBinding(VkDescriptorType type, VkShaderStageFlags stageFlags, uint32_t binding);

    // Create descriptor write for buffer
    VkWriteDescriptorSet writeDescriptorBuffer(VkDescriptorType type, VkDescriptorSet dstSet, VkDescriptorBufferInfo* bufferInfo , uint32_t binding);

    // Create command buffer begin info
    VkCommandBufferBeginInfo commandBufferBeginInfo(VkCommandBufferUsageFlagBits flags);

    // Create sampler info
    VkSamplerCreateInfo samplerCreateInfo(VkFilter filters, VkSamplerAddressMode samplerAddressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT);

    // Create descriptor write for Image
    VkWriteDescriptorSet writeDescriptorImage(VkDescriptorType type, VkDescriptorSet dstSet, VkDescriptorImageInfo* imageInfo, uint32_t binding);

    // Create semaphore
    void initSemaphore(Context& context, VkSemaphore& semaphoreToCreate);
}



#endif //RENDER_VK_INIT_H
