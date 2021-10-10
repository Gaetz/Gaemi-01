//
// Created by gaetz on 08/10/2021.
//

#include "RendererBackEndVulkan.h"

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <SDL2/SDL_vulkan.h>


#include "../../externals/vkbootstrap/VkBootstrap.h"
#include "VkInit.h"
#include "PipelineBuilder.h"
#include "../../math/Transformations.h"
#include "../../math/Functions.h"
#include "../../Locator.h"



using engine::render::vk::PipelineBuilder;
using engine::render::vk::Vertex;

#ifdef _DEBUG
#define VK_CHECK(x)                                                             \
    do                                                                          \
    {                                                                           \
        VkResult err = x;                                                       \
        if (err)                                                                \
        {                                                                       \
            LOG(LogLevel::Error) << "Detected Vulkan error: " << err;           \
            abort();                                                            \
        }                                                                       \
    } while (0)
#else
#define VK_CHECK(x) \
        x
#endif

using engine::render::vk::RendererBackEndVulkan;

bool RendererBackEndVulkan::init(const string& appName, u16 width, u16 height) {
    context.windowExtent.width = width;
    context.windowExtent.height = height;

    context.init(appName);

    initSwapchain();
    initCommands();
    initDefaultRenderpass();
    initFramebuffers();
    initSyncStructures();
    initDescriptors();
    initPipelines();
    loadMeshes();
    loadImages();
    initScene();
    return true;
}

void RendererBackEndVulkan::close() {
    cleanupVulkan();
}

void RendererBackEndVulkan::resize() {

}

void RendererBackEndVulkan::cleanupVulkan() {
    // Make sure the GPU has stopped before clearing
    --frameNumber;
    vkWaitForFences(context.device, 1, &getCurrentFrame().renderFence, true, 1000000000);
    ++frameNumber;

    context.close();
}

void RendererBackEndVulkan::initSwapchain() {

    // -- SWAPCHAIN INIT --

    vkb::SwapchainBuilder swapchainBuilder {context.chosenGPU, context.device, context.surface};
    vkb::Swapchain vkbSwapchain = swapchainBuilder.use_default_format_selection()
            .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
            .set_desired_extent(context.windowExtent.width, context.windowExtent.height)
            .build()
            .value();
    // We use VK_PRESENT_MODE_FIFO_KHR for hard Vsync: the FPS fo the RendererBackEndVulkan will
    // be limited to the refresh speed of the monitor.
    swapchain = vkbSwapchain.swapchain;
    swapchainImages = vkbSwapchain.get_images().value();
    swapchainImageViews = vkbSwapchain.get_image_views().value();
    swapchainImageFormat = vkbSwapchain.image_format;

    // Cleanup callback
    context.mainDeletionQueue.pushFunction([=]() {
        vkDestroySwapchainKHR(context.device, swapchain, nullptr);
    });


    // -- DEPTH BUFFER INIT --

    // Depth size matches window size
    VkExtent3D depthImageExtent {
            context.windowExtent.width,
            context.windowExtent.height,
            1
    };

    // Hardcode depth format to 32 bits float
    depthFormat = VK_FORMAT_D32_SFLOAT;

    VkImageCreateInfo depthImageInfo = render::vk::imageCreateInfo(depthFormat,
                                                                   VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                                                   depthImageExtent);
    // Allocate depth image from local GPU memory
    VmaAllocationCreateInfo depthImageAllocInfo {};
    depthImageAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    depthImageAllocInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    // Allocate and create the image
    vmaCreateImage(context.allocator, &depthImageInfo, &depthImageAllocInfo, &depthImage.image, &depthImage.allocation, nullptr);

    // Build an image-view for the depth image to use for rendering
    VkImageViewCreateInfo depthImageViewInfo = render::vk::imageViewCreateInfo(depthFormat,
                                                                               depthImage.image,
                                                                               VK_IMAGE_ASPECT_DEPTH_BIT);
    VK_CHECK(vkCreateImageView(context.device, &depthImageViewInfo, nullptr, &depthImageView));

    // Cleanup callback
    context.mainDeletionQueue.pushFunction([=]() {
        vkDestroyImageView(context.device, depthImageView, nullptr);
        vmaDestroyImage(context.allocator, depthImage.image, depthImage.allocation);
    });
}

void RendererBackEndVulkan::initCommands() {
    // Command pool
    auto commandPoolInfo = render::vk::commandPoolCreateInfo(context.graphicsQueueFamily,
                                                             VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    for(int i = 0; i < FRAME_OVERLAP; ++i) {
        VK_CHECK(vkCreateCommandPool(context.device, &commandPoolInfo, nullptr, &frames[i].commandPool));

        // Command buffer
        auto cmdAllocInfo = render::vk::commandBufferAllocateInfo(frames[i].commandPool, 1);
        VK_CHECK(vkAllocateCommandBuffers(context.device, &cmdAllocInfo, &frames[i].mainCommandBuffer));
        // Cleanup callback
        context.mainDeletionQueue.pushFunction([=]() {
            vkDestroyCommandPool(context.device, frames[i].commandPool, nullptr);
        });
    }

    // Upload command pool
    auto uploadCommandPoolInfo = render::vk::commandPoolCreateInfo(context.graphicsQueueFamily);
    VK_CHECK(vkCreateCommandPool(context.device, &uploadCommandPoolInfo, nullptr, &uploadContext.commandPool));
    context.mainDeletionQueue.pushFunction([=]() {
        vkDestroyCommandPool(context.device, uploadContext.commandPool, nullptr);
    });
}

void RendererBackEndVulkan::initDefaultRenderpass() {

    // -- COLOR ATTACHMENT --
    // The renderpass will use this color attachment.
    VkAttachmentDescription colorAttachment {};
    // The attachment will have the format needed by the swapchain
    colorAttachment.format = swapchainImageFormat;
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
    depthAttachment.format = depthFormat;
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

    VK_CHECK(vkCreateRenderPass(context.device, &vkRenderPassCreateInfo, nullptr, &renderPass));

    // Cleanup callback
    context.mainDeletionQueue.pushFunction([=]() {
        vkDestroyRenderPass(context.device, renderPass, nullptr);
    });
}

void RendererBackEndVulkan::initFramebuffers() {
    // Create the framebuffers for the swapchain images.
    // This will connect the render-pass to the images for rendering
    VkFramebufferCreateInfo framebufferCreateInfo = {};
    framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferCreateInfo.pNext = nullptr;
    framebufferCreateInfo.renderPass = renderPass;
    framebufferCreateInfo.width = context.windowExtent.width;
    framebufferCreateInfo.height = context.windowExtent.height;
    framebufferCreateInfo.layers = 1;

    // Grab how many images we have in the swapchain
    const uint32_t swapchainImageCount = swapchainImages.size();
    framebuffers = vector<VkFramebuffer>(swapchainImageCount);

    // Create framebuffers for each of the swapchain image views
    for (size_t i = 0; i < swapchainImageCount; ++i) {
        // Prepare attachments
        array<VkImageView, 2> attachments { swapchainImageViews[i], depthImageView };

        // Create framebuffer
        framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferCreateInfo.pAttachments = attachments.data();
        VK_CHECK(vkCreateFramebuffer(context.device, &framebufferCreateInfo, nullptr, &framebuffers[i]));

        // Cleanup callback
        context.mainDeletionQueue.pushFunction([=]() {
            vkDestroyFramebuffer(context.device, framebuffers[i], nullptr);
            vkDestroyImageView(context.device, swapchainImageViews[i], nullptr);
        });
    }
}

void RendererBackEndVulkan::initSyncStructures() {
    // Image fence and semaphores
    VkFenceCreateInfo fenceCreateInfo = render::vk::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
    VkSemaphoreCreateInfo semaphoreCreateInfo = render::vk::semaphoreCreateInfo();

    for (int i = 0; i < FRAME_OVERLAP; ++i) {
        VK_CHECK(vkCreateFence(context.device, &fenceCreateInfo, nullptr, &frames[i].renderFence));
        // Cleanup callback
        context.mainDeletionQueue.pushFunction([=]() {
            vkDestroyFence(context.device, frames[i].renderFence, nullptr);
        });


        VK_CHECK(vkCreateSemaphore(context.device, &semaphoreCreateInfo, nullptr, &frames[i].presentSemaphore));
        VK_CHECK(vkCreateSemaphore(context.device, &semaphoreCreateInfo, nullptr, &frames[i].renderSemaphore));
        // Cleanup callbacks
        context.mainDeletionQueue.pushFunction([=]() {
            vkDestroySemaphore(context.device, frames[i].presentSemaphore, nullptr);
            vkDestroySemaphore(context.device, frames[i].renderSemaphore, nullptr);
        });
    }

    // Upload fence
    VkFenceCreateInfo uploadFenceCreateInfo = render::vk::fenceCreateInfo();
    VK_CHECK(vkCreateFence(context.device, &uploadFenceCreateInfo, nullptr, &uploadContext.uploadFence));
    context.mainDeletionQueue.pushFunction([=]() {
        vkDestroyFence(context.device, uploadContext.uploadFence, nullptr);
    });
}


bool RendererBackEndVulkan::beginFrame(u32 dt) {
    // Wait until the GPU has finished rendering the last frame. Timeout of 1 second
    VK_CHECK(vkWaitForFences(context.device, 1, &getCurrentFrame().renderFence, true, 1000000000));
    VK_CHECK(vkResetFences(context.device, 1, &getCurrentFrame().renderFence));

    // Request image from the swapchain, one second timeout
    swapchainImageIndex = 0;
    VK_CHECK(vkAcquireNextImageKHR(context.device, swapchain, 1000000000, getCurrentFrame().presentSemaphore, nullptr, &swapchainImageIndex));

    // Now that we are sure that the commands finished executing,
    // we can safely reset the command buffer to begin recording again.
    VK_CHECK(vkResetCommandBuffer(getCurrentFrame().mainCommandBuffer, 0));

    // Naming it cmd for shorter writing
    VkCommandBuffer cmd = getCurrentFrame().mainCommandBuffer;

    // Begin the command buffer recording. We will use this command buffer exactly once,
    // so we want to let Vulkan know that
    VkCommandBufferBeginInfo cmdBeginInfo = render::vk::commandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

    // Make a clear-color from frame number. This will flash with a 120*pi frame period.
    VkClearValue clearValue;
    float flash = abs(sin(static_cast<float>(frameNumber) / 120.f));
    clearValue.color = {{0.0f, 0.0f, flash, 1.0f}};

    // Clear depth
    VkClearValue depthClear;
    depthClear.depthStencil.depth = 1.0f;

    // Prepare clear values
    array<VkClearValue, 2> clearValues { clearValue, depthClear };

    // Start the main renderpass.
    // We will use the clear color and depth from above, and the framebuffer of the index the swapchain gave us.
    VkRenderPassBeginInfo renderPassBeginInfo {};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.pNext = nullptr;
    renderPassBeginInfo.renderPass = renderPass;
    renderPassBeginInfo.renderArea.offset.x = 0;
    renderPassBeginInfo.renderArea.offset.y = 0;
    renderPassBeginInfo.renderArea.extent = context.windowExtent;
    renderPassBeginInfo.framebuffer = framebuffers[swapchainImageIndex];
    renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassBeginInfo.pClearValues = clearValues.data();
    vkCmdBeginRenderPass(cmd, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    // -- DRAW HERE --
    drawObjects(cmd, renderables.data(), renderables.size());

    // Finalize the render pass
    vkCmdEndRenderPass(cmd);
    // Finalize the command buffer (we can no longer add commands, but it can now be executed)
    VK_CHECK(vkEndCommandBuffer(cmd));

    // Prepare the submission to the queue.
    // We want to wait on the presentSemaphore, as that semaphore is signaled when the swapchain is ready.
    // We will signal the renderSemaphore, to signal that rendering has finished.
    VkSubmitInfo submitInfo {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;
    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    submitInfo.pWaitDstStageMask = &waitStage;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &getCurrentFrame().presentSemaphore;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &getCurrentFrame().renderSemaphore;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;

    // Submit command buffer to the queue and execute it.
    // renderFence will now block until the graphic commands finish execution
    VK_CHECK(vkQueueSubmit(context.graphicsQueue, 1, &submitInfo, getCurrentFrame().renderFence));

    // This will put the image we just rendered into the visible window.
    // We want to wait on the renderSemaphore for that,
    // as it's necessary that drawing commands have finished before the image is displayed to the user.
    VkPresentInfoKHR presentInfo {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;
    presentInfo.pSwapchains = &swapchain;
    presentInfo.swapchainCount = 1;
    presentInfo.pWaitSemaphores = &getCurrentFrame().renderSemaphore;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pImageIndices = &swapchainImageIndex;

    VK_CHECK(vkQueuePresentKHR(context.graphicsQueue, &presentInfo));
    return true;
}

bool RendererBackEndVulkan::endFrame(u32 dt) {

    return true;
}

bool RendererBackEndVulkan::loadShaderModule(const char* path, VkShaderModule* outShaderModule) {
    // Open the file stream in binary mode and put cursor at end
    std::ifstream file {path, std::ios::ate | std::ios::binary};
    if (!file.is_open()) {
        LOG(LogLevel::Error) << "Cannot read shader file " << path;
        return false;
    }
    // Find what the size of the file is by looking up the location of the cursor, which is end of file.
    auto fileSize = file.tellg();
    // SpirV expects the buffer to be on uint32, so make sure to reserve an int vector big enough for the entire file.
    vector<uint32_t> buffer(fileSize / sizeof(uint32_t));
    // Put file cursor at beginning.
    file.seekg(0);
    // Load the entire file into the buffer.
    file.read((char*) buffer.data(), fileSize);
    file.close();

    // Create a new shader module, using the buffer we loaded
    VkShaderModuleCreateInfo shaderModuleCreateInfo {};
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.pNext = nullptr;
    shaderModuleCreateInfo.codeSize = buffer.size() * sizeof(uint32_t);
    shaderModuleCreateInfo.pCode = buffer.data();

    // Check that the creation goes well.
    VkShaderModule shaderModule;
    if (vkCreateShaderModule(context.device, &shaderModuleCreateInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        return false;
    }
    *outShaderModule = shaderModule;

    return true;
}

AllocatedBuffer RendererBackEndVulkan::createBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage) {
    VkBufferCreateInfo bufferInfo {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext = nullptr;
    bufferInfo.size = allocSize;
    bufferInfo.usage = usage;

    VmaAllocationCreateInfo vmaAllocInfo {};
    vmaAllocInfo.usage = memoryUsage;

    // Allocate memory
    AllocatedBuffer allocatedBuffer {};
    VK_CHECK(vmaCreateBuffer(context.allocator,
                             &bufferInfo,
                             &vmaAllocInfo,
                             &allocatedBuffer.buffer,
                             &allocatedBuffer.allocation,
                             nullptr));

    context.mainDeletionQueue.pushFunction([=](){
        vmaDestroyBuffer(context.allocator, allocatedBuffer.buffer, allocatedBuffer.allocation);
    });
    return allocatedBuffer;
}


size_t RendererBackEndVulkan::padUniformBufferSize(size_t originalSize) const {
    // Calculate required alignment based on minimum device offset alignment
    size_t minUboAlignment = context.gpuProperties.limits.minUniformBufferOffsetAlignment;
    size_t alignedSize = originalSize;
    if (minUboAlignment > 0) {
        alignedSize = (alignedSize + minUboAlignment - 1) & ~(minUboAlignment - 1);
    }
    return alignedSize;
}


void RendererBackEndVulkan::initDescriptors() {
    // Create a descriptor pool that will hold 10 uniform buffers
    vector<VkDescriptorPoolSize> sizes {
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 10 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10 }
    };
    VkDescriptorPoolCreateInfo poolInfo {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = 0;
    poolInfo.maxSets = 10;
    poolInfo.poolSizeCount = static_cast<uint32_t>(sizes.size());
    poolInfo.pPoolSizes = sizes.data();
    VK_CHECK(vkCreateDescriptorPool(context.device, &poolInfo, nullptr, &descriptorPool));
    context.mainDeletionQueue.pushFunction([=](){
        vkDestroyDescriptorPool(context.device, descriptorPool, nullptr);
    });

    // -- UNIFORM BUFFERS for camera and scene --

    // Binding for camera data at 0
    VkDescriptorSetLayoutBinding cameraBufferBinding = render::vk::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            VK_SHADER_STAGE_VERTEX_BIT,
            0);
    // Binding for scene data at 1
    VkDescriptorSetLayoutBinding sceneBufferBinding = render::vk::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            1);

    array<VkDescriptorSetLayoutBinding, 2> bindings { cameraBufferBinding, sceneBufferBinding };

    // Set layout info
    VkDescriptorSetLayoutCreateInfo setInfo {};
    setInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    setInfo.pNext = nullptr;
    setInfo.flags = 0;
    setInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    setInfo.pBindings = bindings.data();
    VK_CHECK(vkCreateDescriptorSetLayout(context.device, &setInfo, nullptr, &globalSetLayout));
    context.mainDeletionQueue.pushFunction([=](){
        vkDestroyDescriptorSetLayout(context.device, globalSetLayout, nullptr);
    });

    // Uniform buffer for scene data: two scene params (one for each frame) in same buffer
    const size_t sceneParamBufferSize = FRAME_OVERLAP * padUniformBufferSize(sizeof(render::vk::GPUSceneData));
    sceneParamsBuffer = createBuffer(sceneParamBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

    // -- STORAGE BUFFER for objects --
    VkDescriptorSetLayoutBinding objectBind = render::vk::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            VK_SHADER_STAGE_VERTEX_BIT,
            0);
    VkDescriptorSetLayoutCreateInfo set2Info;
    set2Info.bindingCount = 1;
    set2Info.flags = 0;
    set2Info.pNext = nullptr;
    set2Info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    set2Info.pBindings = &objectBind;
    VK_CHECK(vkCreateDescriptorSetLayout(context.device, &set2Info, nullptr, &objectSetLayout));
    context.mainDeletionQueue.pushFunction([=](){
        vkDestroyDescriptorSetLayout(context.device, objectSetLayout, nullptr);
    });

    // -- SAMPLER for single texture --
    VkDescriptorSetLayoutBinding textureBind = render::vk::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            0);
    VkDescriptorSetLayoutCreateInfo set3info {};
    set3info.bindingCount = 1;
    set3info.flags = 0;
    set3info.pNext = nullptr;
    set3info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    set3info.pBindings = &textureBind;
    VK_CHECK(vkCreateDescriptorSetLayout(context.device, &set3info, nullptr, &singleTextureSetLayout));
    context.mainDeletionQueue.pushFunction([=](){
        vkDestroyDescriptorSetLayout(context.device, singleTextureSetLayout, nullptr);
    });

    // -- CREATE BUFFERS FOR EACH FRAMES --
    for(int i = 0; i < FRAME_OVERLAP; ++i) {
        frames[i].cameraBuffer = createBuffer(sizeof(render::vk::GPUCameraData),
                                              VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                              VMA_MEMORY_USAGE_CPU_TO_GPU);

        frames[i].objectBuffer = createBuffer(
                sizeof(render::vk::GPUObjectData) * MAX_OBJECTS,
                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                VMA_MEMORY_USAGE_CPU_TO_GPU);

        // Allocate one descriptor for each frame, global set layout
        VkDescriptorSetAllocateInfo allocInfo {};
        allocInfo.pNext = nullptr;
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorSetCount = 1;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.pSetLayouts = &globalSetLayout;
        VK_CHECK(vkAllocateDescriptorSets(context.device, &allocInfo, &frames[i].globalDescriptor));

        // Object set layout
        VkDescriptorSetAllocateInfo objectSetAllocInfo {};
        objectSetAllocInfo.pNext = nullptr;
        objectSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        objectSetAllocInfo.descriptorSetCount = 1;
        objectSetAllocInfo.descriptorPool = descriptorPool;
        objectSetAllocInfo.pSetLayouts = &objectSetLayout;
        VK_CHECK(vkAllocateDescriptorSets(context.device, &objectSetAllocInfo, &frames[i].objectDescriptor));

        // Make the descriptor point into the camera buffer
        VkDescriptorBufferInfo cameraInfo {};
        // Descriptor will point to the camera buffer
        cameraInfo.buffer = frames[i].cameraBuffer.buffer;
        // ...at 0 offset
        cameraInfo.offset = 0;
        // ...of the size of camera data struct
        cameraInfo.range = sizeof(render::vk::GPUCameraData);

        // Make the descriptor point into the scene buffer
        VkDescriptorBufferInfo sceneInfo {};
        sceneInfo.buffer = sceneParamsBuffer.buffer;
        sceneInfo.offset = 0; // For non dynamic buffer, would be: padUniformBufferSize(sizeof(render::vk::GPUSceneData)) * i;
        sceneInfo.range = sizeof(render::vk::GPUSceneData);

        // Make the object descriptor point to the object storage buffer
        VkDescriptorBufferInfo objectBufferInfo {};
        objectBufferInfo.buffer = frames[i].objectBuffer.buffer;
        objectBufferInfo.offset = 0;
        objectBufferInfo.range = sizeof(render::vk::GPUObjectData) * MAX_OBJECTS;

        // Writes
        VkWriteDescriptorSet cameraWrite = render::vk::writeDescriptorBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, frames[i].globalDescriptor, &cameraInfo, 0);
        VkWriteDescriptorSet sceneWrite = render::vk::writeDescriptorBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, frames[i].globalDescriptor, &sceneInfo, 1);
        VkWriteDescriptorSet objectWrite = render::vk::writeDescriptorBuffer(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, frames[i].objectDescriptor, &objectBufferInfo, 0);
        array<VkWriteDescriptorSet, 3> setWrites { cameraWrite, sceneWrite, objectWrite };

        vkUpdateDescriptorSets(context.device, static_cast<uint32_t>(setWrites.size()), setWrites.data(), 0, nullptr);
    }
}



void RendererBackEndVulkan::initPipelines() {
    // -- SHADER MODULES --
    VkShaderModule defaultLitFragShader;
    if (!loadShaderModule("../../shaders/defaultLit.frag.spv", &defaultLitFragShader)) {
        LOG(LogLevel::Error) << "Error when building the default textured fragment shader module";
    } else {
        LOG(LogLevel::Info) << "Default lit fragment shader successfully loaded";
    }

    VkShaderModule meshVertShader;
    if (!loadShaderModule("../../shaders/triMesh.vert.spv", &meshVertShader)) {
        LOG(LogLevel::Error) << "Error when building the triangle mesh vertex shader module";
    } else {
        LOG(LogLevel::Info) << "Mesh vertex shader successfully loaded";
    }


    // -- LAYOUT --
    // The pipeline layout that controls the inputs/outputs of the shader
    // We are not using descriptor sets or other systems yet, so no need to use anything other than empty default
    // Pipeline layout for push constants
    VkPipelineLayoutCreateInfo texturedMeshPipelineLayoutInfo = render::vk::pipelineLayoutCreateInfo();

    // Setup push constants
    VkPushConstantRange pushConstant;
    pushConstant.offset = 0;
    pushConstant.size = sizeof(render::vk::MeshPushConstants);
    pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    texturedMeshPipelineLayoutInfo.pushConstantRangeCount = 1;
    texturedMeshPipelineLayoutInfo.pPushConstantRanges = &pushConstant;

    array<VkDescriptorSetLayout, 3> setLayouts { globalSetLayout, objectSetLayout, singleTextureSetLayout };
    texturedMeshPipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
    texturedMeshPipelineLayoutInfo.pSetLayouts = setLayouts.data();

    VK_CHECK(vkCreatePipelineLayout(context.device, &texturedMeshPipelineLayoutInfo, nullptr, &texturedMeshPipelineLayout));


    // -- BUILD PIPELINE --
    PipelineBuilder pipelineBuilder;

    // Clear the shader stages for the builder
    pipelineBuilder.shaderStages.clear();

    // Shader stage
    pipelineBuilder.shaderStages.push_back(
            render::vk::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, meshVertShader));
    pipelineBuilder.shaderStages.push_back(
            render::vk::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, defaultLitFragShader));

    // Vertex input controls how to read vertices from vertex buffers.
    pipelineBuilder.vertexInputInfo = render::vk::vertexInputStateCreateInfo();
    render::vk::VertexInputDescription vertexDescription = Vertex::getVertexDescription();

    // Connect the pipeline builder vertex input info to the one we get from Vertex
    pipelineBuilder.vertexInputInfo.pVertexAttributeDescriptions = vertexDescription.attributes.data();
    pipelineBuilder.vertexInputInfo.vertexAttributeDescriptionCount = vertexDescription.attributes.size();
    pipelineBuilder.vertexInputInfo.pVertexBindingDescriptions = vertexDescription.bindings.data();
    pipelineBuilder.vertexInputInfo.vertexBindingDescriptionCount = vertexDescription.bindings.size();

    // Input assembly is the configuration for drawing triangle lists, strips, or individual points.
    pipelineBuilder.inputAssembly = render::vk::inputAssemblyCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

    // Build viewport and scissor from the swapchain extents
    pipelineBuilder.viewport.x = 0.0f;
    pipelineBuilder.viewport.y = 0.0f;
    pipelineBuilder.viewport.width = (float) context.windowExtent.width;
    pipelineBuilder.viewport.height = (float) context.windowExtent.height;
    pipelineBuilder.viewport.minDepth = 0.0f;
    pipelineBuilder.viewport.maxDepth = 1.0f;

    pipelineBuilder.scissor.offset = {0, 0};
    pipelineBuilder.scissor.extent = context.windowExtent;

    // Configure the rasterizer to draw filled triangles
    pipelineBuilder.rasterizer = render::vk::rasterizationStateCreateInfo(VK_POLYGON_MODE_FILL);

    // Multisampling
    pipelineBuilder.multisampling = render::vk::multisamplingStateCreateInfo();

    // Blend attachments
    pipelineBuilder.colorBlendAttachment = render::vk::colorBlendAttachmentState();

    // Depth Stencil
    pipelineBuilder.depthStencil = render::vk::depthStencilCreateInfo(true, true, VK_COMPARE_OP_LESS_OR_EQUAL);

    // Build pipeline & material
    pipelineBuilder.pipelineLayout = texturedMeshPipelineLayout;
    meshPipeline = pipelineBuilder.buildPipeline(context.device, renderPass);
    createMaterial(meshPipeline, texturedMeshPipelineLayout, "defaultMesh");


    // -- CLEANUP --
    vkDestroyShaderModule(context.device, defaultLitFragShader, nullptr);
    vkDestroyShaderModule(context.device, meshVertShader, nullptr);

    context.mainDeletionQueue.pushFunction([=]() {
        vkDestroyPipeline(context.device, meshPipeline, nullptr);
        vkDestroyPipelineLayout(context.device, texturedMeshPipelineLayout, nullptr);
    });
}

void RendererBackEndVulkan::initScene() {
    // Our scene will be composed of a monkey and triangles

    // Monkey
    RenderObject monkey {};
    monkey.mesh = getMesh("monkey");
    monkey.material = getMaterial("defaultMesh");
    monkey.transform = Mat4 {1.0f };
    renderables.push_back(monkey);

    // Triangles
    for (int x = -20; x <= 20; ++x) {
        for (int y = -20; y <= 20; ++y) {
            RenderObject triangle {};
            triangle.mesh = getMesh("triangle");
            triangle.material = getMaterial("defaultMesh");
            Mat4 translation = math::translate(Mat4{1.0f}, Vec3{x, 0, y});
            Mat4 scale = math::scale(Mat4{1.0f}, Vec3{0.2f, 0.2f, 0.2f});
            triangle.transform = translation * scale;
            renderables.push_back(triangle);
        }
    }

    // Add lost empire
    RenderObject map;
    map.mesh = getMesh("lostEmpire");
    map.material = getMaterial("defaultMesh");
    map.transform = math::translate(Mat4{1.f}, Vec3{ 5,-10,0 });
    renderables.push_back(map);

    // Textures for lost empire
    VkSamplerCreateInfo samplerInfo = render::vk::samplerCreateInfo(VK_FILTER_NEAREST);
    VkSampler texSampler;
    vkCreateSampler(context.device, &samplerInfo, nullptr, &texSampler);
    context.mainDeletionQueue.pushFunction([=]() {
        vkDestroySampler(context.device, texSampler, nullptr);
    });
    render::vk::Material* texturedMat =	getMaterial("defaultMesh");

    // Allocate the descriptor set for single-texture to use on the material
    VkDescriptorSetAllocateInfo allocInfo {};
    allocInfo.pNext = nullptr;
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &singleTextureSetLayout;
    vkAllocateDescriptorSets(context.device, &allocInfo, &texturedMat->textureSet);

    // Write to the descriptor set so that it points to our empire_diffuse texture
    VkDescriptorImageInfo imageBufferInfo;
    imageBufferInfo.sampler = texSampler;
    imageBufferInfo.imageView = textures["empire_diffuse"].imageView;
    imageBufferInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    VkWriteDescriptorSet texture1 = render::vk::writeDescriptorImage(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, texturedMat->textureSet, &imageBufferInfo, 0);
    vkUpdateDescriptorSets(context.device, 1, &texture1, 0, nullptr);
}

void RendererBackEndVulkan::loadMeshes() {
    // Load triangle
    Mesh triangleMesh;
    triangleMesh.vertices.resize(3);
    triangleMesh.vertices[0].position = { 1.f, 1.f, 0.f };
    triangleMesh.vertices[1].position = { -1.f, 1.f, 0.f };
    triangleMesh.vertices[2].position = { 0.f, -1.f, 0.f };
    triangleMesh.vertices[0].color = { 0.f, 1.f, 0.f };
    triangleMesh.vertices[1].color = { 0.f, 1.f, 0.f };
    triangleMesh.vertices[2].color = { 0.f, 1.f, 0.f };

    // Load monkey from obj file
    Mesh monkeyMesh;
    monkeyMesh.loadFromObj("../../assets/monkey_smooth.obj");

    // Lost empire
    Mesh lostEmpire;
    lostEmpire.loadFromObj("../../assets/lost_empire.obj");

    uploadMesh(triangleMesh);
    uploadMesh(monkeyMesh);
    uploadMesh(lostEmpire);

    // Store (copy) meshes in lists
    meshes["triangle"] = triangleMesh;
    meshes["monkey"] = monkeyMesh;
    meshes["lostEmpire"] = lostEmpire;
}

void RendererBackEndVulkan::uploadMesh(Mesh& mesh) {
    const size_t bufferSize = mesh.vertices.size() * sizeof(Vertex);

    // -- STAGING BUFFER to load mesh into CPU memory --
    VkBufferCreateInfo stagingBufferInfo {};
    stagingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    stagingBufferInfo.pNext = nullptr;
    stagingBufferInfo.size = bufferSize;
    stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    // Let the VMA library know that this data should be writeable by CPU only
    VmaAllocationCreateInfo vmaAllocInfo {};
    vmaAllocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

    // Allocate memory
    AllocatedBuffer stagingBuffer {};
    VK_CHECK(vmaCreateBuffer(context.allocator,
                             &stagingBufferInfo,
                             &vmaAllocInfo,
                             &stagingBuffer.buffer,
                             &stagingBuffer.allocation,
                             nullptr));

    // Copy data into staging buffer
    void* data;
    vmaMapMemory(context.allocator, stagingBuffer.allocation, &data);
    memcpy(data, mesh.vertices.data(), mesh.vertices.size() * sizeof(Vertex));
    vmaUnmapMemory(context.allocator, stagingBuffer.allocation);

    // -- VERTEX BUFFER --
    // Transfer data from staging buffer to GPU vertex buffer
    VkBufferCreateInfo vertexBufferInfo {};
    vertexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vertexBufferInfo.pNext = nullptr;
    vertexBufferInfo.size = bufferSize;
    vertexBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    // Let the VMA library know that this data should be GPU native
    vmaAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    // Allocate the buffer
    VK_CHECK(vmaCreateBuffer(context.allocator,
                             &vertexBufferInfo,
                             &vmaAllocInfo,
                             &mesh.vertexBuffer.buffer,
                             &mesh.vertexBuffer.allocation,
                             nullptr));

    // -- EXECUTE ALLOC COMMAND --
    immediateSubmit([=](VkCommandBuffer cmd) {
        VkBufferCopy copy;
        copy.srcOffset = 0;
        copy.dstOffset = 0;
        copy.size = bufferSize;
        vkCmdCopyBuffer(cmd, stagingBuffer.buffer, mesh.vertexBuffer.buffer, 1, &copy);
    });

    // -- CLEAN --
    context.mainDeletionQueue.pushFunction([=]() {
        vmaDestroyBuffer(context.allocator, mesh.vertexBuffer.buffer, mesh.vertexBuffer.allocation);
    });
    vmaDestroyBuffer(context.allocator, stagingBuffer.buffer, stagingBuffer.allocation);
}

engine::render::vk::Material*
RendererBackEndVulkan::createMaterial(VkPipeline pipelineP, VkPipelineLayout pipelineLayoutP, const string& name) {
    engine::render::vk::Material material {};
    material.pipeline = pipelineP;
    material.pipelineLayout = pipelineLayoutP;
    materials[name] = material;
    return &materials[name];
}

engine::render::vk::Material* RendererBackEndVulkan::getMaterial(const string& name) {
    auto it = materials.find(name);
    if (it == end(materials)) {
        return nullptr;
    } else {
        return &(*it).second;
    }
}

engine::render::vk::Mesh* RendererBackEndVulkan::getMesh(const string& name) {
    auto it = meshes.find(name);
    if (it == end(meshes)) {
        return nullptr;
    } else {
        return &(*it).second;
    }
}

void RendererBackEndVulkan::drawObjects(VkCommandBuffer cmd, RenderObject* first, size_t count) {
    // View and projection
    Vec3 camPos {0.f, -6.f, -10.f};
    Mat4 view = math::translate(Mat4{1.f}, camPos);
    Mat4 projection = math::perspective(math::toRad(70.f),
                                        static_cast<float>(context.windowExtent.width) / static_cast<float>(context.windowExtent.height),
                                        0.1f, 200.f);
    projection[1][1] *= -1;

    // Fill GPUCamera data struct and copy it to the buffer
    render::vk::GPUCameraData cameraData {};
    cameraData.projection = projection;
    cameraData.view = view;
    cameraData.viewProj = projection * view;
    void* data;
    vmaMapMemory(context.allocator, getCurrentFrame().cameraBuffer.allocation, &data);
    memcpy(data, &cameraData, sizeof(render::vk::GPUCameraData));
    vmaUnmapMemory(context.allocator, getCurrentFrame().cameraBuffer.allocation);

    // Fill GPUSceneData with random ambient color
    float framed = static_cast<float>(frameNumber) / 120.0f;
    sceneParams.ambientColor = { math::sin(framed), 0, math::cos(framed), 1};

    char* sceneData;
    vmaMapMemory(context.allocator, sceneParamsBuffer.allocation, (void**)&sceneData);
    unsigned int frameIndex = frameNumber % FRAME_OVERLAP;
    sceneData += padUniformBufferSize(sizeof(render::vk::GPUSceneData)) * frameIndex;
    memcpy(sceneData, &sceneParams, sizeof(render::vk::GPUSceneData));
    vmaUnmapMemory(context.allocator, sceneParamsBuffer.allocation);

    // Fill storage buffer with object model matrices
    void* objectData;
    vmaMapMemory(context.allocator, getCurrentFrame().objectBuffer.allocation, &objectData);
    render::vk::GPUObjectData* objectSSBO = (render::vk::GPUObjectData*)objectData;
    for (int i = 0; i < count; ++i) {
        RenderObject& object = first[i];
        objectSSBO[i].modelMatrix = object.transform;
    }
    vmaUnmapMemory(context.allocator, getCurrentFrame().objectBuffer.allocation);

    // Draw with push constants
    Mesh* lastMesh = nullptr;
    render::vk::Material* lastMaterial = nullptr;
    for (int i = 0; i < count; ++i) {
        RenderObject& object = first[i];

        // Bind pipeline if material is different
        if (object.material != lastMaterial) {
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, object.material->pipeline);
            lastMaterial = object.material;

            // -- CAMERA DESCRIPTOR --
            // Offset for scene data dynamic buffer
            uint32_t uniformOffset = padUniformBufferSize(sizeof(render::vk::GPUSceneData)) * frameIndex;
            // Bind descriptor set when changing pipeline
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, object.material->pipelineLayout, 0, 1, &getCurrentFrame().globalDescriptor, 1, &uniformOffset);

            // -- OBJECT DATA DESCRIPTOR --
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, object.material->pipelineLayout, 1, 1, &getCurrentFrame().objectDescriptor, 0, nullptr);

            // -- TEXTURE --
            if (object.material->textureSet != VK_NULL_HANDLE) {
                // Texture descriptor
                vkCmdBindDescriptorSets(cmd,
                                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                                        object.material->pipelineLayout,
                                        2, 1, &object.material->textureSet,
                                        0, nullptr);
            }
        }

        // Push transform
        render::vk::MeshPushConstants constants {};
        constants.renderMatrix = object.transform;
        vkCmdPushConstants(cmd,
                           object.material->pipelineLayout,
                           VK_SHADER_STAGE_VERTEX_BIT,
                           0, sizeof(render::vk::MeshPushConstants),
                           &constants);

        // Bind mesh if mesh is different
        if (object.mesh != lastMesh) {
            VkDeviceSize offset = 0;
            vkCmdBindVertexBuffers(cmd, 0, 1, &object.mesh->vertexBuffer.buffer, &offset);
            lastMesh = object.mesh;
        }

        // Draw
        vkCmdDraw(cmd, object.mesh->vertices.size(), 1, 0, i);
    }
}

void RendererBackEndVulkan::immediateSubmit(std::function<void(VkCommandBuffer)>&& submittedFunc) {
    VkCommandBufferAllocateInfo cmdAllocateInfo = render::vk::commandBufferAllocateInfo(uploadContext.commandPool, 1);
    VkCommandBuffer cmd;
    VK_CHECK(vkAllocateCommandBuffers(context.device, &cmdAllocateInfo, &cmd));

    VkCommandBufferBeginInfo cmdBeginInfo = render::vk::commandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

    // Execute function
    submittedFunc(cmd);
    VK_CHECK(vkEndCommandBuffer(cmd));

    VkSubmitInfo submitInfo {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;
    submitInfo.waitSemaphoreCount = 0;
    submitInfo.pWaitSemaphores = nullptr;
    submitInfo.pWaitDstStageMask = nullptr;
    submitInfo.commandBufferCount = 1;
    submitInfo.signalSemaphoreCount = 0;
    submitInfo.pSignalSemaphores = nullptr;
    submitInfo.pCommandBuffers = &cmd;

    // Submit command buffer to the queue and execute it.
    // uploadFence will now block until the graphic commands finish execution
    VK_CHECK(vkQueueSubmit(context.graphicsQueue, 1, &submitInfo, uploadContext.uploadFence));
    vkWaitForFences(context.device, 1, &uploadContext.uploadFence, true, 9999999999);
    vkResetFences(context.device, 1, &uploadContext.uploadFence);

    // Clear the command pool. This will free the command buffer too
    vkResetCommandPool(context.device, uploadContext.commandPool, 0);
}

void RendererBackEndVulkan::loadImages() {
    render::vk::Texture lostEmpire {};
    loadImageFromFile("../../assets/lost_empire-RGBA.png", lostEmpire.image);

    VkImageViewCreateInfo imageViewInfo = render::vk::imageViewCreateInfo(VK_FORMAT_R8G8B8A8_SRGB, lostEmpire.image.image, VK_IMAGE_ASPECT_COLOR_BIT);
    vkCreateImageView(context.device, &imageViewInfo, nullptr, &lostEmpire.imageView);
    context.mainDeletionQueue.pushFunction([=]() {
        vkDestroyImageView(context.device, lostEmpire.imageView, nullptr);
    });

    textures["empire_diffuse"] = lostEmpire;
}

bool RendererBackEndVulkan::loadImageFromFile(const string& path, render::vk::AllocatedImage& outImage)
{
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    if (!pixels) {
        LOG(LogLevel::Error) << "Failed to load texture file " << path;
        return false;
    }
    void* pixel_ptr = pixels;
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB;
    AllocatedBuffer stagingBuffer = createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

    void* data;
    vmaMapMemory(context.allocator, stagingBuffer.allocation, &data);
    memcpy(data, pixel_ptr, static_cast<size_t>(imageSize));
    vmaUnmapMemory(context.allocator, stagingBuffer.allocation);
    stbi_image_free(pixels);

    VkExtent3D imageExtent;
    imageExtent.width = static_cast<uint32_t>(texWidth);
    imageExtent.height = static_cast<uint32_t>(texHeight);
    imageExtent.depth = 1;

    render::vk::AllocatedImage newImage {};
    VkImageCreateInfo imageCreateInfo = render::vk::imageCreateInfo(imageFormat, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, imageExtent);
    VmaAllocationCreateInfo imageAllocInfo {};
    imageAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    // Allocate and create the image
    vmaCreateImage(context.allocator, &imageCreateInfo, &imageAllocInfo, &newImage.image, &newImage.allocation, nullptr);

    // Transition image to transfer-receiver
    immediateSubmit([&](VkCommandBuffer cmd) {
        VkImageSubresourceRange range;
        range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        range.baseMipLevel = 0;
        range.levelCount = 1;
        range.baseArrayLayer = 0;
        range.layerCount = 1;

        VkImageMemoryBarrier imageBarrierToTransfer {};
        imageBarrierToTransfer.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageBarrierToTransfer.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageBarrierToTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imageBarrierToTransfer.image = newImage.image;
        imageBarrierToTransfer.subresourceRange = range;
        imageBarrierToTransfer.srcAccessMask = 0;
        imageBarrierToTransfer.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        // Barrier the image into the transfer-receive layout
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrierToTransfer);

        VkBufferImageCopy copyRegion {};
        copyRegion.bufferOffset = 0;
        copyRegion.bufferRowLength = 0;
        copyRegion.bufferImageHeight = 0;

        copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copyRegion.imageSubresource.mipLevel = 0;
        copyRegion.imageSubresource.baseArrayLayer = 0;
        copyRegion.imageSubresource.layerCount = 1;
        copyRegion.imageExtent = imageExtent;

        // Copy the buffer into the image
        vkCmdCopyBufferToImage(cmd, stagingBuffer.buffer, newImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

        VkImageMemoryBarrier imageBarrierToReadable = imageBarrierToTransfer;
        imageBarrierToReadable.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imageBarrierToReadable.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageBarrierToReadable.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        imageBarrierToReadable.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        // Barrier the image into the shader readable layout
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrierToReadable);
    });

    context.mainDeletionQueue.pushFunction([=]() {
        vmaDestroyImage(context.allocator, newImage.image, newImage.allocation);
    });

    LOG(LogLevel::Info) << "Texture loaded successfully " << path;
    outImage = newImage;
    return true;
}

