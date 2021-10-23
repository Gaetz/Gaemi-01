//
// Created by gaetz on 10/10/2021.
//

#include "Swapchain.h"
#include "Init.h"
#include "../../../../externals/vkbootstrap/VkBootstrap.h"

using engine::render::vk::Swapchain;
using engine::render::vk::Context;

Swapchain::Swapchain(Context &contextP) : context { contextP } {}


void Swapchain::init() {
    create();
}

void engine::render::vk::Swapchain::reinit() {
    destroy();
    create();
}

void engine::render::vk::Swapchain::create() {
    // -- SWAPCHAIN INIT --
    vkb::SwapchainBuilder swapchainBuilder { context.chosenGPU, context.device, context.surface };
    vkb::Swapchain vkbSwapchain = swapchainBuilder.use_default_format_selection()
            .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
            .set_desired_extent(context.windowExtent.width, context.windowExtent.height)
            .build()
            .value();
    // We use VK_PRESENT_MODE_FIFO_KHR for hard Vsync: the FPS fo the RendererBackEndVulkan will
    // be limited to the refresh speed of the monitor.
    handle = vkbSwapchain.swapchain;
    images = vkbSwapchain.get_images().value();
    imageViews = vkbSwapchain.get_image_views().value();
    imageFormat = vkbSwapchain.image_format;

    // Cleanup callback
    destroyHandle = [=]() { vkDestroySwapchainKHR(context.device, handle, nullptr); };


    // -- DEPTH BUFFER INIT --
    // Depth size matches window size
    VkExtent3D depthImageExtent {
            context.windowExtent.width,
            context.windowExtent.height,
            1
    };

    // Hardcode depth format to 32 bits float
    depthFormat = VK_FORMAT_D32_SFLOAT;

    VkImageCreateInfo depthImageInfo = imageCreateInfo(depthFormat,
                                                       VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                                       depthImageExtent);
    // Allocate depth image from local GPU memory
    VmaAllocationCreateInfo depthImageAllocInfo {};
    depthImageAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    depthImageAllocInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    // Allocate and create the image
    vmaCreateImage(context.allocator, &depthImageInfo, &depthImageAllocInfo, &depthImage.image, &depthImage.allocation,
                   nullptr);

    // Build an image-view for the depth image to use for rendering
    VkImageViewCreateInfo depthImageViewInfo = imageViewCreateInfo(depthFormat,
                                                                   depthImage.image,
                                                                   VK_IMAGE_ASPECT_DEPTH_BIT);
    VK_CHECK(vkCreateImageView(context.device, &depthImageViewInfo, nullptr, &depthImageView));

    // Cleanup callback
    destroyDepthImageViews = [=]() { vkDestroyImageView(context.device, depthImageView, nullptr); };
    destroyDepthImages = [=]() { vmaDestroyImage(context.allocator, depthImage.image, depthImage.allocation); };

    // DELETION QUEUE REGISTERING
    // If we reInit swapchain, the same function will be called with different function pointers.
    // We do not need to add it again.
    if (!deletionQueueRegistered) {
        context.mainDeletionQueue.pushFunction([=]() {
            destroy();
        });
        deletionQueueRegistered = true;
    }
}

void engine::render::vk::Swapchain::destroy() {
    destroyDepthImages();
    destroyDepthImageViews();
    destroyHandle();
}

void Swapchain::acquireNextImage(u64 timeout, VkSemaphore semaphore, VkFence fence, u32 &outImageIndex) {
    VK_SWAP_CHECK(vkAcquireNextImageKHR(context.device, handle, 1000000000, semaphore, fence, &outImageIndex));
}

void Swapchain::present(VkSemaphore renderCompleteSemaphore, u32 &imageIndex) {
    // This will put the image we just rendered into the visible window.
    // We want to wait on the renderSemaphore for that,
    // as it's necessary that drawing commands have finished before the image is displayed to the user.
    VkPresentInfoKHR presentInfo {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;
    presentInfo.pSwapchains = &handle;
    presentInfo.swapchainCount = 1;
    presentInfo.pWaitSemaphores = &renderCompleteSemaphore;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pImageIndices = &imageIndex;

    VK_SWAP_CHECK(vkQueuePresentKHR(context.graphicsQueue, &presentInfo));
}

