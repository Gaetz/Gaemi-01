#include "Engine.h"
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include <SDL2/SDL.h>
#include <fstream>
#include "../../externals/vkbootstrap/VkBootstrap.h"
#include <array>

#include "vk/VkInit.h"
#include "Timer.h"
#include "../Log.h"
#include "vk/PipelineBuilder.h"
#include "math/Transformations.h"
#include "math/Functions.h"
#include "Memory.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

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

using engine::Engine;
using engine::vk::PipelineBuilder;
using engine::input::InputState;
using engine::vk::Vertex;
using std::array;
using engine::Memory;

engine::EngineState Engine::state {};

engine::EngineState& Engine::getState() {
    return state;
}

GAPI Engine::Engine(const EngineConfig& configP) :
    config { configP },
    windowExtent { configP.startWidth, configP.startHeight },
    inputSystem { configP.startWidth, configP.startHeight }
{}

void Engine::run() {
    Timer timer;
    state.game->load();
    while (state.isRunning) {
        u32 time = getAbsoluteTime();
        u32 dt = timer.computeDeltaTime(time);

        // Input
        const InputState& inputState = processInputs();
        state.game->setInputState(inputState);

        // Update loop
        update(dt);
        state.game->update(dt);

        // Render
        draw();
        state.game->draw();

        // Frame delay is managed by the renderer,
        // which synchronizes with the monitor framerate.
    }
}

void Engine::init(Game& game) {
    Memory::instance().init();
    state.game = &game;
    state.platform->init(config.name, config.startPositionX, config.startPositionY, config.startWidth, config.startHeight);
    inputSystem.init();
    state.isInitialized = true;

    initVulkan();
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

    state.game->load();
    state.isRunning = true;
    state.isPaused = false;
}

void Engine::cleanup() {
    if (state.isInitialized) {
        cleanupVulkan();
        state.game->cleanup();
        state.platform->shutdown();
        Memory::instance().close();
    }
}

InputState engine::Engine::processInputs() {
    inputSystem.preUpdate();
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        state.isRunning = inputSystem.processEvent(event);
    }
    inputSystem.update();

    return inputSystem.getInputState();
}

void Engine::update(u64 dt) {
    state.platform->update(dt);
    state.game->update(dt);
}

u32 Engine::getAbsoluteTime() const {
    return state.platform->getAbsoluteTimeMs();
}

f64 Engine::getAbsoluteTimeSeconds() const {
    return state.platform->getAbsoluteTimeSeconds();
}

void Engine::sleep(u64 ms) const {
    state.platform->sleep(ms);
}

void Engine::draw() {
    // Wait until the GPU has finished rendering the last frame. Timeout of 1 second
    VK_CHECK(vkWaitForFences(device, 1, &getCurrentFrame().renderFence, true, 1000000000));
    VK_CHECK(vkResetFences(device, 1, &getCurrentFrame().renderFence));

    // Request image from the swapchain, one second timeout
    uint32_t swapchainImageIndex;
    VK_CHECK(vkAcquireNextImageKHR(device, swapchain, 1000000000, getCurrentFrame().presentSemaphore, nullptr, &swapchainImageIndex));

    // Now that we are sure that the commands finished executing,
    // we can safely reset the command buffer to begin recording again.
    VK_CHECK(vkResetCommandBuffer(getCurrentFrame().mainCommandBuffer, 0));

    // Naming it cmd for shorter writing
    VkCommandBuffer cmd = getCurrentFrame().mainCommandBuffer;

    // Begin the command buffer recording. We will use this command buffer exactly once,
    // so we want to let Vulkan know that
    VkCommandBufferBeginInfo cmdBeginInfo = vk::commandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

    // Make a clear-color from frame number. This will flash with a 120*pi frame period.
    VkClearValue clearValue;
    float flash = abs(sin(static_cast<float>(state.frameNumber) / 120.f));
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
    renderPassBeginInfo.renderArea.extent = windowExtent;
    renderPassBeginInfo.framebuffer = framebuffers[swapchainImageIndex];
    renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassBeginInfo.pClearValues = clearValues.data();
    vkCmdBeginRenderPass(cmd, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    // -- DRAW HERE --
    //game.draw();
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
    VK_CHECK(vkQueueSubmit(graphicsQueue, 1, &submitInfo, getCurrentFrame().renderFence));

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

    VK_CHECK(vkQueuePresentKHR(graphicsQueue, &presentInfo));

    // Game update
    state.game->draw();

    // Increase the number of frames drawn
    state.frameNumber++;
}

void Engine::initVulkan() {
    // -- INSTANCE --
    vkb::InstanceBuilder builder;

#ifdef _DEBUG
    // Make vulkan instance with basic debug features
    auto instanceResult = builder.set_app_name("Gaemi")
            .request_validation_layers(true)
            .require_api_version(1, 1, 0)
            .use_default_debug_messenger()
            .build();
    vkb::Instance vkbInstance = instanceResult.value();
    instance = vkbInstance.instance;
    debugMessenger = vkbInstance.debug_messenger;

#else
    auto instanceResult = builder.set_app_name("Gaemi")
            .request_validation_layers(false)
            .require_api_version(1, 1, 0)
            .build();
    vkb::Instance vkbInstance = instanceResult.value();
    instance = vkbInstance.instance;

#endif

    // -- DEVICE --
    // get the surface of the window we opened with SDL
    SDL_Vulkan_CreateSurface((SDL_Window*)state.platform->getScreenSurface(), instance, &surface);

    // Use VkBootstrap to select a GPU.
    // We want a GPU that can write to the SDL surface and supports Vulkan 1.1
    vkb::PhysicalDeviceSelector selector {vkbInstance};
    vkb::PhysicalDevice physicalDevice = selector
            .set_minimum_version(1, 1)
            .set_surface(surface)
            .select()
            .value();

    // Create the final Vulkan device
    vkb::DeviceBuilder deviceBuilder {physicalDevice};
    vkb::Device vkbDevice = deviceBuilder.build().value();

    // Get the VkDevice handle used in the rest of a Vulkan application
    device = vkbDevice.device;
    chosenGPU = physicalDevice.physical_device;

    // Get graphics queue
    graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
    graphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

    // Physical device properties
    vkGetPhysicalDeviceProperties(chosenGPU, &gpuProperties);
	LOG(LogLevel::Trace) << "The GPU has a minimum buffer alignment of " << gpuProperties.limits.minUniformBufferOffsetAlignment;

    // Vulkan memory allocator
    VmaAllocatorCreateInfo allocatorCreateInfo {};
    allocatorCreateInfo.physicalDevice = chosenGPU;
    allocatorCreateInfo.device = device;
    allocatorCreateInfo.instance = instance;
    vmaCreateAllocator(&allocatorCreateInfo, &allocator);
    mainDeletionQueue.pushFunction([=](){
        vmaDestroyAllocator(allocator);
    });
}

void Engine::cleanupVulkan() {
    // Make sure the GPU has stopped before clearing
    --state.frameNumber;
    vkWaitForFences(device, 1, &getCurrentFrame().renderFence, true, 1000000000);
    ++state.frameNumber;
    mainDeletionQueue.flush();

    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyDevice(device, nullptr);
#ifdef _DEBUG
    vkb::destroy_debug_utils_messenger(instance, debugMessenger);
#endif
    vkDestroyInstance(instance, nullptr);
}

void Engine::initSwapchain() {

    // -- SWAPCHAIN INIT --

    vkb::SwapchainBuilder swapchainBuilder {chosenGPU, device, surface};
    vkb::Swapchain vkbSwapchain = swapchainBuilder.use_default_format_selection()
            .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
            .set_desired_extent(windowExtent.width, windowExtent.height)
            .build()
            .value();
    // We use VK_PRESENT_MODE_FIFO_KHR for hard Vsync: the FPS fo the engine will
    // be limited to the refresh speed of the monitor.
    swapchain = vkbSwapchain.swapchain;
    swapchainImages = vkbSwapchain.get_images().value();
    swapchainImageViews = vkbSwapchain.get_image_views().value();
    swapchainImageFormat = vkbSwapchain.image_format;

    // Cleanup callback
    mainDeletionQueue.pushFunction([=]() {
        vkDestroySwapchainKHR(device, swapchain, nullptr);
    });


    // -- DEPTH BUFFER INIT --

    // Depth size matches window size
    VkExtent3D depthImageExtent {
        windowExtent.width,
        windowExtent.height,
        1
    };

    // Hardcode depth format to 32 bits float
    depthFormat = VK_FORMAT_D32_SFLOAT;

    VkImageCreateInfo depthImageInfo = vk::imageCreateInfo(depthFormat,
                                                            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                                            depthImageExtent);
    // Allocate depth image from local GPU memory
    VmaAllocationCreateInfo depthImageAllocInfo {};
    depthImageAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    depthImageAllocInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    // Allocate and create the image
    vmaCreateImage(allocator, &depthImageInfo, &depthImageAllocInfo, &depthImage.image, &depthImage.allocation, nullptr);

    // Build an image-view for the depth image to use for rendering
    VkImageViewCreateInfo depthImageViewInfo = vk::imageViewCreateInfo(depthFormat,
                                                                       depthImage.image,
                                                                       VK_IMAGE_ASPECT_DEPTH_BIT);
    VK_CHECK(vkCreateImageView(device, &depthImageViewInfo, nullptr, &depthImageView));

    // Cleanup callback
    mainDeletionQueue.pushFunction([=]() {
        vkDestroyImageView(device, depthImageView, nullptr);
        vmaDestroyImage(allocator, depthImage.image, depthImage.allocation);
    });
}

void Engine::initCommands() {
    // Command pool
    auto commandPoolInfo = vk::commandPoolCreateInfo(graphicsQueueFamily,
                                                     VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    for(int i = 0; i < FRAME_OVERLAP; ++i) {
        VK_CHECK(vkCreateCommandPool(device, &commandPoolInfo, nullptr, &frames[i].commandPool));

        // Command buffer
        auto cmdAllocInfo = vk::commandBufferAllocateInfo(frames[i].commandPool, 1);
        VK_CHECK(vkAllocateCommandBuffers(device, &cmdAllocInfo, &frames[i].mainCommandBuffer));
        // Cleanup callback
        mainDeletionQueue.pushFunction([=]() {
            vkDestroyCommandPool(device, frames[i].commandPool, nullptr);
        });
    }

    // Upload command pool
    auto uploadCommandPoolInfo = vk::commandPoolCreateInfo(graphicsQueueFamily);
    VK_CHECK(vkCreateCommandPool(device, &uploadCommandPoolInfo, nullptr, &uploadContext.commandPool));
    mainDeletionQueue.pushFunction([=]() {
        vkDestroyCommandPool(device, uploadContext.commandPool, nullptr);
    });
}

void Engine::initDefaultRenderpass() {

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

    VK_CHECK(vkCreateRenderPass(device, &vkRenderPassCreateInfo, nullptr, &renderPass));

    // Cleanup callback
    mainDeletionQueue.pushFunction([=]() {
        vkDestroyRenderPass(device, renderPass, nullptr);
    });
}

void Engine::initFramebuffers() {
    // Create the framebuffers for the swapchain images.
    // This will connect the render-pass to the images for rendering
    VkFramebufferCreateInfo framebufferCreateInfo = {};
    framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferCreateInfo.pNext = nullptr;
    framebufferCreateInfo.renderPass = renderPass;
    framebufferCreateInfo.width = windowExtent.width;
    framebufferCreateInfo.height = windowExtent.height;
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
        VK_CHECK(vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr, &framebuffers[i]));

        // Cleanup callback
        mainDeletionQueue.pushFunction([=]() {
            vkDestroyFramebuffer(device, framebuffers[i], nullptr);
            vkDestroyImageView(device, swapchainImageViews[i], nullptr);
        });
    }
}

void Engine::initSyncStructures() {
    // Image fence and semaphores
    VkFenceCreateInfo fenceCreateInfo = vk::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
    VkSemaphoreCreateInfo semaphoreCreateInfo = vk::semaphoreCreateInfo();

	for (int i = 0; i < FRAME_OVERLAP; ++i) {
        VK_CHECK(vkCreateFence(device, &fenceCreateInfo, nullptr, &frames[i].renderFence));
        // Cleanup callback
        mainDeletionQueue.pushFunction([=]() {
            vkDestroyFence(device, frames[i].renderFence, nullptr);
        });


        VK_CHECK(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &frames[i].presentSemaphore));
        VK_CHECK(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &frames[i].renderSemaphore));
        // Cleanup callbacks
        mainDeletionQueue.pushFunction([=]() {
            vkDestroySemaphore(device, frames[i].presentSemaphore, nullptr);
            vkDestroySemaphore(device, frames[i].renderSemaphore, nullptr);
        });
    }

	// Upload fence
    VkFenceCreateInfo uploadFenceCreateInfo = vk::fenceCreateInfo();
    VK_CHECK(vkCreateFence(device, &uploadFenceCreateInfo, nullptr, &uploadContext.uploadFence));
    mainDeletionQueue.pushFunction([=]() {
        vkDestroyFence(device, uploadContext.uploadFence, nullptr);
    });
}

bool Engine::loadShaderModule(const char* path, VkShaderModule* outShaderModule) {
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
    if (vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        return false;
    }
    *outShaderModule = shaderModule;

    return true;
}

AllocatedBuffer Engine::createBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage) {
    VkBufferCreateInfo bufferInfo {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext = nullptr;
    bufferInfo.size = allocSize;
    bufferInfo.usage = usage;

    VmaAllocationCreateInfo vmaAllocInfo {};
    vmaAllocInfo.usage = memoryUsage;

    // Allocate memory
    AllocatedBuffer allocatedBuffer {};
    VK_CHECK(vmaCreateBuffer(allocator,
                             &bufferInfo,
                             &vmaAllocInfo,
                             &allocatedBuffer.buffer,
                             &allocatedBuffer.allocation,
                             nullptr));

    mainDeletionQueue.pushFunction([=](){
        vmaDestroyBuffer(allocator, allocatedBuffer.buffer, allocatedBuffer.allocation);
    });
    return allocatedBuffer;
}


size_t Engine::padUniformBufferSize(size_t originalSize) const {
    // Calculate required alignment based on minimum device offset alignment
	size_t minUboAlignment = gpuProperties.limits.minUniformBufferOffsetAlignment;
	size_t alignedSize = originalSize;
	if (minUboAlignment > 0) {
		alignedSize = (alignedSize + minUboAlignment - 1) & ~(minUboAlignment - 1);
	}
	return alignedSize;
}


void Engine::initDescriptors() {
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
    VK_CHECK(vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool));
    mainDeletionQueue.pushFunction([=](){
        vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    });

    // -- UNIFORM BUFFERS for camera and scene --

    // Binding for camera data at 0
    VkDescriptorSetLayoutBinding cameraBufferBinding = vk::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_SHADER_STAGE_VERTEX_BIT, 0);
	// Binding for scene data at 1
	VkDescriptorSetLayoutBinding sceneBufferBinding = vk::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 1);

    array<VkDescriptorSetLayoutBinding, 2> bindings { cameraBufferBinding, sceneBufferBinding };

    // Set layout info
    VkDescriptorSetLayoutCreateInfo setInfo {};
    setInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    setInfo.pNext = nullptr;
    setInfo.flags = 0;
    setInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    setInfo.pBindings = bindings.data();
    VK_CHECK(vkCreateDescriptorSetLayout(device, &setInfo, nullptr, &globalSetLayout));
    mainDeletionQueue.pushFunction([=](){
        vkDestroyDescriptorSetLayout(device, globalSetLayout, nullptr);
    });

    // Uniform buffer for scene data: two scene params (one for each frame) in same buffer
    const size_t sceneParamBufferSize = FRAME_OVERLAP * padUniformBufferSize(sizeof(vk::GPUSceneData));
    sceneParamsBuffer = createBuffer(sceneParamBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

    // -- STORAGE BUFFER for objects --
    VkDescriptorSetLayoutBinding objectBind = vk::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0);
    VkDescriptorSetLayoutCreateInfo set2Info;
    set2Info.bindingCount = 1;
    set2Info.flags = 0;
    set2Info.pNext = nullptr;
    set2Info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    set2Info.pBindings = &objectBind;
    VK_CHECK(vkCreateDescriptorSetLayout(device, &set2Info, nullptr, &objectSetLayout));
    mainDeletionQueue.pushFunction([=](){
        vkDestroyDescriptorSetLayout(device, objectSetLayout, nullptr);
    });

    // -- SAMPLER for single texture --
    VkDescriptorSetLayoutBinding textureBind = vk::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0);
    VkDescriptorSetLayoutCreateInfo set3info {};
    set3info.bindingCount = 1;
    set3info.flags = 0;
    set3info.pNext = nullptr;
    set3info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    set3info.pBindings = &textureBind;
    VK_CHECK(vkCreateDescriptorSetLayout(device, &set3info, nullptr, &singleTextureSetLayout));
    mainDeletionQueue.pushFunction([=](){
        vkDestroyDescriptorSetLayout(device, singleTextureSetLayout, nullptr);
    });

    // -- CREATE BUFFERS FOR EACH FRAMES --
    for(int i = 0; i < FRAME_OVERLAP; ++i) {
        frames[i].cameraBuffer = createBuffer(sizeof(vk::GPUCameraData),
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VMA_MEMORY_USAGE_CPU_TO_GPU);

        frames[i].objectBuffer = createBuffer(sizeof(vk::GPUObjectData) * MAX_OBJECTS, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

        // Allocate one descriptor for each frame, global set layout
        VkDescriptorSetAllocateInfo allocInfo {};
        allocInfo.pNext = nullptr;
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorSetCount = 1;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.pSetLayouts = &globalSetLayout;
        VK_CHECK(vkAllocateDescriptorSets(device, &allocInfo, &frames[i].globalDescriptor));

        // Object set layout
        VkDescriptorSetAllocateInfo objectSetAllocInfo {};
        objectSetAllocInfo.pNext = nullptr;
        objectSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        objectSetAllocInfo.descriptorSetCount = 1;
        objectSetAllocInfo.descriptorPool = descriptorPool;
        objectSetAllocInfo.pSetLayouts = &objectSetLayout;
        VK_CHECK(vkAllocateDescriptorSets(device, &objectSetAllocInfo, &frames[i].objectDescriptor));

        // Make the descriptor point into the camera buffer
        VkDescriptorBufferInfo cameraInfo {};
        // Descriptor will point to the camera buffer
        cameraInfo.buffer = frames[i].cameraBuffer.buffer;
        // ...at 0 offset
        cameraInfo.offset = 0;
        // ...of the size of camera data struct
        cameraInfo.range = sizeof(vk::GPUCameraData);

        // Make the descriptor point into the scene buffer
        VkDescriptorBufferInfo sceneInfo {};
        sceneInfo.buffer = sceneParamsBuffer.buffer;
        sceneInfo.offset = 0; // For non dynamic buffer, would be: padUniformBufferSize(sizeof(vk::GPUSceneData)) * i;
        sceneInfo.range = sizeof(vk::GPUSceneData);

        // Make the object descriptor point to the object storage buffer
        VkDescriptorBufferInfo objectBufferInfo {};
        objectBufferInfo.buffer = frames[i].objectBuffer.buffer;
        objectBufferInfo.offset = 0;
        objectBufferInfo.range = sizeof(vk::GPUObjectData) * MAX_OBJECTS;

        // Writes
        VkWriteDescriptorSet cameraWrite = vk::writeDescriptorBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, frames[i].globalDescriptor, &cameraInfo, 0);
        VkWriteDescriptorSet sceneWrite = vk::writeDescriptorBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, frames[i].globalDescriptor, &sceneInfo, 1);
        VkWriteDescriptorSet objectWrite = vk::writeDescriptorBuffer(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, frames[i].objectDescriptor, &objectBufferInfo, 0);
        array<VkWriteDescriptorSet, 3> setWrites { cameraWrite, sceneWrite, objectWrite };

        vkUpdateDescriptorSets(device, static_cast<uint32_t>(setWrites.size()), setWrites.data(), 0, nullptr);
    }
}



void Engine::initPipelines() {
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
    VkPipelineLayoutCreateInfo texturedMeshPipelineLayoutInfo = vk::pipelineLayoutCreateInfo();

    // Setup push constants
    VkPushConstantRange pushConstant;
    pushConstant.offset = 0;
    pushConstant.size = sizeof(vk::MeshPushConstants);
    pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    texturedMeshPipelineLayoutInfo.pushConstantRangeCount = 1;
    texturedMeshPipelineLayoutInfo.pPushConstantRanges = &pushConstant;

    array<VkDescriptorSetLayout, 3> setLayouts { globalSetLayout, objectSetLayout, singleTextureSetLayout };
    texturedMeshPipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
    texturedMeshPipelineLayoutInfo.pSetLayouts = setLayouts.data();

    VK_CHECK(vkCreatePipelineLayout(device, &texturedMeshPipelineLayoutInfo, nullptr, &texturedMeshPipelineLayout));


    // -- BUILD PIPELINE --
    PipelineBuilder pipelineBuilder;

    // Clear the shader stages for the builder
    pipelineBuilder.shaderStages.clear();

    // Shader stage
    pipelineBuilder.shaderStages.push_back(
            vk::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, meshVertShader));
    pipelineBuilder.shaderStages.push_back(
            vk::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, defaultLitFragShader));

    // Vertex input controls how to read vertices from vertex buffers.
    pipelineBuilder.vertexInputInfo = vk::vertexInputStateCreateInfo();
    vk::VertexInputDescription vertexDescription = Vertex::getVertexDescription();

    // Connect the pipeline builder vertex input info to the one we get from Vertex
    pipelineBuilder.vertexInputInfo.pVertexAttributeDescriptions = vertexDescription.attributes.data();
    pipelineBuilder.vertexInputInfo.vertexAttributeDescriptionCount = vertexDescription.attributes.size();
    pipelineBuilder.vertexInputInfo.pVertexBindingDescriptions = vertexDescription.bindings.data();
    pipelineBuilder.vertexInputInfo.vertexBindingDescriptionCount = vertexDescription.bindings.size();

    // Input assembly is the configuration for drawing triangle lists, strips, or individual points.
    pipelineBuilder.inputAssembly = vk::inputAssemblyCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

    // Build viewport and scissor from the swapchain extents
    pipelineBuilder.viewport.x = 0.0f;
    pipelineBuilder.viewport.y = 0.0f;
    pipelineBuilder.viewport.width = (float) windowExtent.width;
    pipelineBuilder.viewport.height = (float) windowExtent.height;
    pipelineBuilder.viewport.minDepth = 0.0f;
    pipelineBuilder.viewport.maxDepth = 1.0f;

    pipelineBuilder.scissor.offset = {0, 0};
    pipelineBuilder.scissor.extent = windowExtent;

    // Configure the rasterizer to draw filled triangles
    pipelineBuilder.rasterizer = vk::rasterizationStateCreateInfo(VK_POLYGON_MODE_FILL);

    // Multisampling
    pipelineBuilder.multisampling = vk::multisamplingStateCreateInfo();

    // Blend attachments
    pipelineBuilder.colorBlendAttachment = vk::colorBlendAttachmentState();

    // Depth Stencil
    pipelineBuilder.depthStencil = vk::depthStencilCreateInfo(true, true, VK_COMPARE_OP_LESS_OR_EQUAL);

    // Build pipeline & material
    pipelineBuilder.pipelineLayout = texturedMeshPipelineLayout;
    meshPipeline = pipelineBuilder.buildPipeline(device, renderPass);
    createMaterial(meshPipeline, texturedMeshPipelineLayout, "defaultMesh");


    // -- CLEANUP --
    vkDestroyShaderModule(device, defaultLitFragShader, nullptr);
    vkDestroyShaderModule(device, meshVertShader, nullptr);

    mainDeletionQueue.pushFunction([=]() {
        vkDestroyPipeline(device, meshPipeline, nullptr);
        vkDestroyPipelineLayout(device, texturedMeshPipelineLayout, nullptr);
    });
}

void Engine::initScene() {
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
    VkSamplerCreateInfo samplerInfo = vk::samplerCreateInfo(VK_FILTER_NEAREST);
    VkSampler texSampler;
    vkCreateSampler(device, &samplerInfo, nullptr, &texSampler);
    mainDeletionQueue.pushFunction([=]() {
        vkDestroySampler(device, texSampler, nullptr);
    });
    vk::Material* texturedMat =	getMaterial("defaultMesh");

    // Allocate the descriptor set for single-texture to use on the material
    VkDescriptorSetAllocateInfo allocInfo {};
    allocInfo.pNext = nullptr;
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &singleTextureSetLayout;
    vkAllocateDescriptorSets(device, &allocInfo, &texturedMat->textureSet);

    // Write to the descriptor set so that it points to our empire_diffuse texture
    VkDescriptorImageInfo imageBufferInfo;
    imageBufferInfo.sampler = texSampler;
    imageBufferInfo.imageView = textures["empire_diffuse"].imageView;
    imageBufferInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    VkWriteDescriptorSet texture1 = vk::writeDescriptorImage(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, texturedMat->textureSet, &imageBufferInfo, 0);
    vkUpdateDescriptorSets(device, 1, &texture1, 0, nullptr);
}

void engine::Engine::loadMeshes() {
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

void engine::Engine::uploadMesh(Mesh& mesh) {
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
    VK_CHECK(vmaCreateBuffer(allocator,
                             &stagingBufferInfo,
                             &vmaAllocInfo,
                             &stagingBuffer.buffer,
                             &stagingBuffer.allocation,
                             nullptr));

    // Copy data into staging buffer
    void* data;
    vmaMapMemory(allocator, stagingBuffer.allocation, &data);
    memcpy(data, mesh.vertices.data(), mesh.vertices.size() * sizeof(Vertex));
    vmaUnmapMemory(allocator, stagingBuffer.allocation);

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
    VK_CHECK(vmaCreateBuffer(allocator,
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
    mainDeletionQueue.pushFunction([=]() {
        vmaDestroyBuffer(allocator, mesh.vertexBuffer.buffer, mesh.vertexBuffer.allocation);
    });
    vmaDestroyBuffer(allocator, stagingBuffer.buffer, stagingBuffer.allocation);
}

engine::vk::Material*
engine::Engine::createMaterial(VkPipeline pipelineP, VkPipelineLayout pipelineLayoutP, const string& name) {
    engine::vk::Material material {};
    material.pipeline = pipelineP;
    material.pipelineLayout = pipelineLayoutP;
    materials[name] = material;
    return &materials[name];
}

engine::vk::Material* Engine::getMaterial(const string& name) {
    auto it = materials.find(name);
    if (it == end(materials)) {
        return nullptr;
    } else {
        return &(*it).second;
    }
}

Mesh* Engine::getMesh(const string& name) {
    auto it = meshes.find(name);
    if (it == end(meshes)) {
        return nullptr;
    } else {
        return &(*it).second;
    }
}

void Engine::drawObjects(VkCommandBuffer cmd, RenderObject* first, size_t count) {
    // View and projection
    Vec3 camPos {0.f, -6.f, -10.f};
    Mat4 view = math::translate(Mat4{1.f}, camPos);
    Mat4 projection = math::perspective(math::toRad(70.f),
                                        static_cast<float>(windowExtent.width) / static_cast<float>(windowExtent.height),
                                        0.1f, 200.f);
    projection[1][1] *= -1;

    // Fill GPUCamera data struct and copy it to the buffer
    vk::GPUCameraData cameraData {};
    cameraData.projection = projection;
    cameraData.view = view;
    cameraData.viewProj = projection * view;
    void* data;
    vmaMapMemory(allocator, getCurrentFrame().cameraBuffer.allocation, &data);
    memcpy(data, &cameraData, sizeof(vk::GPUCameraData));
    vmaUnmapMemory(allocator, getCurrentFrame().cameraBuffer.allocation);

    // Fill GPUSceneData with random ambient color
    float framed = static_cast<float>(state.frameNumber) / 120.0f;
    sceneParams.ambientColor = { math::sin(framed), 0, math::cos(framed), 1};

    char* sceneData;
    vmaMapMemory(allocator, sceneParamsBuffer.allocation, (void**)&sceneData);
    unsigned int frameIndex = state.frameNumber % FRAME_OVERLAP;
	sceneData += padUniformBufferSize(sizeof(vk::GPUSceneData)) * frameIndex;
	memcpy(sceneData, &sceneParams, sizeof(vk::GPUSceneData));
	vmaUnmapMemory(allocator, sceneParamsBuffer.allocation);

    // Fill storage buffer with object model matrices
    void* objectData;
    vmaMapMemory(allocator, getCurrentFrame().objectBuffer.allocation, &objectData);
    vk::GPUObjectData* objectSSBO = (vk::GPUObjectData*)objectData;
    for (int i = 0; i < count; ++i) {
        RenderObject& object = first[i];
        objectSSBO[i].modelMatrix = object.transform;
    }
    vmaUnmapMemory(allocator, getCurrentFrame().objectBuffer.allocation);

    // Draw with push constants
    Mesh* lastMesh = nullptr;
    vk::Material* lastMaterial = nullptr;
    for (int i = 0; i < count; ++i) {
        RenderObject& object = first[i];

        // Bind pipeline if material is different
        if (object.material != lastMaterial) {
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, object.material->pipeline);
            lastMaterial = object.material;

            // -- CAMERA DESCRIPTOR --
            // Offset for scene data dynamic buffer
            uint32_t uniformOffset = padUniformBufferSize(sizeof(vk::GPUSceneData)) * frameIndex;
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
        vk::MeshPushConstants constants {};
        constants.renderMatrix = object.transform;
        vkCmdPushConstants(cmd,
                           object.material->pipelineLayout,
                           VK_SHADER_STAGE_VERTEX_BIT,
                           0, sizeof(vk::MeshPushConstants),
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

void engine::Engine::immediateSubmit(std::function<void(VkCommandBuffer)>&& submittedFunc) {
    VkCommandBufferAllocateInfo cmdAllocateInfo = vk::commandBufferAllocateInfo(uploadContext.commandPool, 1);
    VkCommandBuffer cmd;
    VK_CHECK(vkAllocateCommandBuffers(device, &cmdAllocateInfo, &cmd));

    VkCommandBufferBeginInfo cmdBeginInfo = vk::commandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
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
    VK_CHECK(vkQueueSubmit(graphicsQueue, 1, &submitInfo, uploadContext.uploadFence));
    vkWaitForFences(device, 1, &uploadContext.uploadFence, true, 9999999999);
    vkResetFences(device, 1, &uploadContext.uploadFence);

    // Clear the command pool. This will free the command buffer too
    vkResetCommandPool(device, uploadContext.commandPool, 0);
}

void engine::Engine::loadImages() {
    vk::Texture lostEmpire {};
    loadImageFromFile("../../assets/lost_empire-RGBA.png", lostEmpire.image);

    VkImageViewCreateInfo imageViewInfo = vk::imageViewCreateInfo(VK_FORMAT_R8G8B8A8_SRGB, lostEmpire.image.image, VK_IMAGE_ASPECT_COLOR_BIT);
    vkCreateImageView(device, &imageViewInfo, nullptr, &lostEmpire.imageView);
    mainDeletionQueue.pushFunction([=]() {
        vkDestroyImageView(device, lostEmpire.imageView, nullptr);
    });

    textures["empire_diffuse"] = lostEmpire;
}

bool engine::Engine::loadImageFromFile(const string& path, vk::AllocatedImage& outImage)
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
    vmaMapMemory(allocator, stagingBuffer.allocation, &data);
    memcpy(data, pixel_ptr, static_cast<size_t>(imageSize));
    vmaUnmapMemory(allocator, stagingBuffer.allocation);
    stbi_image_free(pixels);

    VkExtent3D imageExtent;
    imageExtent.width = static_cast<uint32_t>(texWidth);
    imageExtent.height = static_cast<uint32_t>(texHeight);
    imageExtent.depth = 1;

    vk::AllocatedImage newImage {};
    VkImageCreateInfo imageCreateInfo = vk::imageCreateInfo(imageFormat, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, imageExtent);
    VmaAllocationCreateInfo imageAllocInfo {};
    imageAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    // Allocate and create the image
    vmaCreateImage(allocator, &imageCreateInfo, &imageAllocInfo, &newImage.image, &newImage.allocation, nullptr);

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

    mainDeletionQueue.pushFunction([=]() {
        vmaDestroyImage(allocator, newImage.image, newImage.allocation);
    });

    LOG(LogLevel::Info) << "Texture loaded successfully " << path;
    outImage = newImage;
    return true;
}

std::array<char, 19> engine::Engine::getDate() {
    return state.platform->getDate();
}
