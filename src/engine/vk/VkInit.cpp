//
// Created by gaetz on 08/06/2021.
//

#include "VkInit.h"

VkCommandPoolCreateInfo engine::vk::commandPoolCreateInfo(uint32_t queueFamilyIndex,
                                                  VkCommandPoolCreateFlags flags) {
    VkCommandPoolCreateInfo commandPoolInfo {};
    commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolInfo.pNext = nullptr;

    commandPoolInfo.queueFamilyIndex = queueFamilyIndex;
    commandPoolInfo.flags = flags;
    return commandPoolInfo;
}

VkCommandBufferAllocateInfo engine::vk::commandBufferAllocateInfo(VkCommandPool pool,
                                                          uint32_t count,
                                                          VkCommandBufferLevel level) {
    VkCommandBufferAllocateInfo cmdAllocInfo {};
    cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdAllocInfo.pNext = nullptr;

    cmdAllocInfo.commandPool = pool;
    cmdAllocInfo.commandBufferCount = count;
    cmdAllocInfo.level = level;
    return cmdAllocInfo;
}

VkPipelineShaderStageCreateInfo engine::vk::pipelineShaderStageCreateInfo(VkShaderStageFlagBits shaderStage,
                                                                  VkShaderModule shaderModule) {
    VkPipelineShaderStageCreateInfo info {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    info.pNext = nullptr;

    info.stage = shaderStage;
    info.module = shaderModule;
    // Entry point of the shader
    info.pName = "main";
    return info;
}

VkPipelineVertexInputStateCreateInfo engine::vk::vertexInputStateCreateInfo() {
    VkPipelineVertexInputStateCreateInfo info {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    info.pNext = nullptr;

    // No vertex bindings or attributes
    info.vertexBindingDescriptionCount = 0;
    info.vertexAttributeDescriptionCount = 0;
    return info;
}

VkPipelineInputAssemblyStateCreateInfo engine::vk::inputAssemblyCreateInfo(VkPrimitiveTopology topology) {
    VkPipelineInputAssemblyStateCreateInfo info {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    info.pNext = nullptr;

    info.topology = topology;
    // We are not going to use primitive restart so leave it on false
    info.primitiveRestartEnable = VK_FALSE;
    return info;
}

VkPipelineRasterizationStateCreateInfo engine::vk::rasterizationStateCreateInfo(VkPolygonMode polygonMode)
{
    VkPipelineRasterizationStateCreateInfo info {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    info.pNext = nullptr;

    info.depthClampEnable = VK_FALSE;
    // Discards all primitives before the rasterization stage if enabled which we don't want.
    // You might enable this, for example, if you’re only interested in the side effects of the vertex
    // processing stages, such as writing to a buffer which you later read from.
    info.rasterizerDiscardEnable = VK_FALSE;
    info.polygonMode = polygonMode;
    info.lineWidth = 1.0f;
    // No backface cull
    info.cullMode = VK_CULL_MODE_NONE;
    info.frontFace = VK_FRONT_FACE_CLOCKWISE;
    // No depth bias
    info.depthBiasEnable = VK_FALSE;
    info.depthBiasConstantFactor = 0.0f;
    info.depthBiasClamp = 0.0f;
    info.depthBiasSlopeFactor = 0.0f;

    return info;
}

VkPipelineMultisampleStateCreateInfo engine::vk::multisamplingStateCreateInfo()
{
    VkPipelineMultisampleStateCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    info.pNext = nullptr;

    info.sampleShadingEnable = VK_FALSE;
    // Multisampling defaulted to no multisampling (1 sample per pixel)
    info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    info.minSampleShading = 1.0f;
    info.pSampleMask = nullptr;
    info.alphaToCoverageEnable = VK_FALSE;
    info.alphaToOneEnable = VK_FALSE;
    return info;
}

VkPipelineColorBlendAttachmentState engine::vk::colorBlendAttachmentState() {
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    return colorBlendAttachment;
}

VkPipelineLayoutCreateInfo engine::vk::pipelineLayoutCreateInfo() {
    VkPipelineLayoutCreateInfo info {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    info.pNext = nullptr;

    //empty defaults
    info.flags = 0;
    info.setLayoutCount = 0;
    info.pSetLayouts = nullptr;
    info.pushConstantRangeCount = 0;
    info.pPushConstantRanges = nullptr;
    return info;
}