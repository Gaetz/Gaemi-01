//
// Created by gaetz on 23/10/2021.
//

#include "RenderPass.h"
#include "Framebuffer.h"

#include <array>
using std::array;


using engine::render::vk::RenderPass;

RenderPass::RenderPass(Context &contextP, const Vec4 &renderZoneP, const Vec4 &clearColorP, f32 depthP, u32 stencilP) :
        context { contextP },
        renderZone { renderZoneP },
        clearColor { clearColorP },
        depth { depthP },
        stencil { stencilP } {}

void RenderPass::init(const Swapchain& swapchain) {
    // -- COLOR ATTACHMENT --
    // The renderpass will use this color attachment.
    VkAttachmentDescription colorAttachment {};
    // The attachment will have the format needed by the swapchain
    colorAttachment.format = swapchain.imageFormat;
    // 1 sample, we won't be doing MSAA
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    // We Clear when this attachment is loaded
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    // We keep the attachment stored when the renderpass ends
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    // We don't care about stencil
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    // We don't know or care about the starting layout of the attachment
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    // After the renderpass ends, the image has to be on a layout ready for display
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // -- COLOR ATTACHMENT REFERENCE --
    VkAttachmentReference colorAttachmentRef {};
    // Attachment number will index into the pAttachments array in the parent renderpass itself
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // -- DEPTH ATTACHMENT --
    VkAttachmentDescription depthAttachment {};
    depthAttachment.flags = 0;
    depthAttachment.format = swapchain.depthFormat;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // -- DEPTH ATTACHMENT REFERENCE --
    VkAttachmentReference depthAttachmentRef {};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // -- SUBPASS --
    VkSubpassDescription subpass {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;
    // Input from a shader
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = nullptr;
    // Used for multisampling color attachments
    subpass.pResolveAttachments = nullptr;
    // £Attachments not used in this subpass but preserved for the next
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = nullptr;

    // -- RENDER PASS DEPENDENCIES --
    VkSubpassDependency dependency;
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;    // First and only subpass
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency.dependencyFlags = 0;

    // -- RENDERPASS --
    // Prepare attachments
    array<VkAttachmentDescription, 2> attachments { colorAttachment, depthAttachment };
    // Setup renderpass
    VkRenderPassCreateInfo vkRenderPassCreateInfo {};
    vkRenderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    // Connect the color attachment to the info
    vkRenderPassCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    vkRenderPassCreateInfo.pAttachments = attachments.data();
    // Connect the subpass to the info
    vkRenderPassCreateInfo.subpassCount = 1;
    vkRenderPassCreateInfo.pSubpasses = &subpass;
    // Connect the dependencies to the info
    vkRenderPassCreateInfo.dependencyCount = 1;
    vkRenderPassCreateInfo.pDependencies = &dependency;

    VK_CHECK(vkCreateRenderPass(context.device, &vkRenderPassCreateInfo, nullptr, &handle));

    // Cleanup callback
    context.mainDeletionQueue.pushFunction([=]() {
        destroy();
    });
}

void RenderPass::destroy() {
    vkDestroyRenderPass(context.device, handle, nullptr);
    handle = nullptr;
}

void RenderPass::begin(CommandBuffer &commandBuffer, const Framebuffer& framebuffer) {
    // Make a clear-color from frame number. This will flash with a 120*pi frame period.
    VkClearValue clearValue;
    clearValue.color = { clearColor.r, clearColor.g, clearColor.b, clearColor.a };

    // Clear depth
    VkClearValue depthClear;
    depthClear.depthStencil.depth = depth;
    depthClear.depthStencil.stencil = stencil;

    // Prepare clear values
    array<VkClearValue, 2> clearValues { clearValue, depthClear };

    // Start the main renderpass.
    // We will use the clear color and depth from above, and the framebuffer of the index the swapchain gave us.
    VkRenderPassBeginInfo renderPassBeginInfo {};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.pNext = nullptr;
    renderPassBeginInfo.renderPass = handle;
    renderPassBeginInfo.renderArea.offset.x = 0;
    renderPassBeginInfo.renderArea.offset.y = 0;
    renderPassBeginInfo.renderArea.extent = context.windowExtent;
    renderPassBeginInfo.framebuffer = framebuffer.handle;
    renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassBeginInfo.pClearValues = clearValues.data();
    vkCmdBeginRenderPass(commandBuffer.handle, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    commandBuffer.state = CommandBufferState::InRenderPass;
}

void RenderPass::end(CommandBuffer &commandBuffer) {
    vkCmdEndRenderPass(commandBuffer.handle);
    commandBuffer.state = CommandBufferState::Recording;
}




