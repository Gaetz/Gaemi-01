//
// Created by gaetz on 26/10/2021.
//

#include "Pipeline.h"
#include "Context.h"
#include "Renderpass.h"
#include "Init.h"
#include <array>

using engine::render::vk::PipelineBuilder;
using engine::render::vk::Pipeline;
using std::array;

VkPipeline PipelineBuilder::buildPipeline(const Context &context, const Renderpass &pass, bool isWireframe) {
    // Make viewport state from our stored viewport and scissor.
    VkPipelineViewportStateCreateInfo viewportState {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.pNext = nullptr;

    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    // Dynamic state
    // Not used for now, serve to ignore init config and set values during pipeline live
    /*
    const u32 dynamicStateCount { 3 };
    array<VkDynamicState, dynamicStateCount> dynamicStates { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_LINE_WIDTH };
    VkPipelineDynamicStateCreateInfo dynamicStateInfo {};
    dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateInfo.dynamicStateCount = dynamicStates.size();
    dynamicStateInfo.pDynamicStates = dynamicStates.data();
     */

    // Configure the rasterizer to draw filled triangles
    VkPolygonMode polygonMode = isWireframe ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
    rasterizer = render::vk::rasterizationStateCreateInfo(polygonMode);

    // Setup color blending
    VkPipelineColorBlendStateCreateInfo colorBlending {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.pNext = nullptr;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    // Build the actual pipeline
    VkGraphicsPipelineCreateInfo pipelineInfo {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.pNext = nullptr;

    pipelineInfo.stageCount = shaderStages.size();
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = pass.handle;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.pDepthStencilState = &depthStencil;
    //pipelineInfo.pDynamicState = &dynamicStateInfo;
    //pipelineInfo.pTessellationState = nullptr;

    // It's easy to error out on create graphics pipeline, so we handle it a bit better than the common VK_CHECK case
    VkPipeline handle;
    if (vkCreateGraphicsPipelines(context.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &handle) !=
        VK_SUCCESS) {
        LOG(LogLevel::Error) << "Failed to create pipeline";
        return VK_NULL_HANDLE;
    } else {
        return handle;
    }
}

void Pipeline::init(PipelineBuilder &builder, const Context& context, const Renderpass& renderpass, bool isWireframe) {
    contextDevice = context.device;
    handle = builder.buildPipeline(context, renderpass, isWireframe);
    layoutHandle = builder.pipelineLayout;

}

void engine::render::vk::Pipeline::destroy() {
    vkDestroyPipeline(contextDevice, handle, nullptr);
    vkDestroyPipelineLayout(contextDevice, layoutHandle, nullptr);
    handle = nullptr;
    layoutHandle = nullptr;
}

void Pipeline::bind(const CommandBuffer &commandBuffer, VkPipelineBindPoint bindPoint) const {
    vkCmdBindPipeline(commandBuffer.handle, bindPoint, handle);
}

