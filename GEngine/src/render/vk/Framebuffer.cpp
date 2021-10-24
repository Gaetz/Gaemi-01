//
// Created by gaetz on 23/10/2021.
//

#include "Framebuffer.h"
#include "Renderpass.h"
#include "Swapchain.h"

using engine::render::vk::Framebuffer;

Framebuffer::Framebuffer(Context &contextP, Renderpass &renderpassP) : context { contextP }, renderpass { renderpassP }, attachments {} {

}

void Framebuffer::init(VkImageView imageView, VkImageView depthImageView) {
    // Create the framebuffers for the swapchain images.
    // This will connect the render-pass to the images for rendering
    VkFramebufferCreateInfo framebufferCreateInfo = {};
    framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferCreateInfo.pNext = nullptr;
    framebufferCreateInfo.renderPass = renderpass.handle;
    framebufferCreateInfo.width = context.windowExtent.width;
    framebufferCreateInfo.height = context.windowExtent.height;
    framebufferCreateInfo.layers = 1;

    // Prepare attachments
    array<VkImageView, 2> attachments { imageView, depthImageView };

    // Create framebuffer
    framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebufferCreateInfo.pAttachments = attachments.data();
    VK_CHECK(vkCreateFramebuffer(context.device, &framebufferCreateInfo, nullptr, &handle));

    // Cleanup callback
    context.mainDeletionQueue.pushFunction([=]() {
        destroy();
    });
}

void Framebuffer::destroy() {
    vkDestroyFramebuffer(context.device, handle, nullptr);
    handle = 0;
    // ImageView destruction is responsability of the swapchain
}
