#include "Engine.h"

#ifdef __linux__

#include <SDL2/SDL.h>

#elif _WIN32
#include <SDL.h>
#endif

#include <fstream>
#include "../../externals/vkbootstrap/VkBootstrap.h"

#include "vk/VkInit.h"
#include "Timer.h"
#include "../Log.h"
#include "vk/VkPipelineBuilder.h"

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
#endif

using engine::Engine;
using engine::vk::VkPipelineBuilder;
using engine::input::InputState;

Engine::Engine() :
        isInitialized {false},
        frameNumber {0},
        windowExtent {1280, 720},
        window {"Gaemi-01"},
        inputSystem {windowExtent.width, windowExtent.height},
        selectedShader {0} {

}

void Engine::init() {
    SDL_Init(SDL_INIT_VIDEO);
    window.init(windowExtent.width, windowExtent.height, false);
    inputSystem.init();
    initVulkan();
    initSwapchain();
    initCommands();
    initDefaultRenderpass();
    initFramebuffers();
    initSyncStructures();
    initPipelines();
    isInitialized = true;
}

void Engine::cleanup() {
    if (isInitialized) {
        cleanupVulkan();
        game.cleanup();
        window.cleanup();
        SDL_Quit();
    }

}

void Engine::draw() {
    // Wait until the GPU has finished rendering the last frame. Timeout of 1 second
    VK_CHECK(vkWaitForFences(device, 1, &renderFence, true, 1000000000));
    VK_CHECK(vkResetFences(device, 1, &renderFence));

    // Request image from the swapchain, one second timeout
    uint32_t swapchainImageIndex;
    VK_CHECK(vkAcquireNextImageKHR(device, swapchain, 1000000000, presentSemaphore, nullptr, &swapchainImageIndex));

    // Now that we are sure that the commands finished executing,
    // we can safely reset the command buffer to begin recording again.
    VK_CHECK(vkResetCommandBuffer(mainCommandBuffer, 0));

    // Naming it cmd for shorter writing
    VkCommandBuffer cmd = mainCommandBuffer;

    // Begin the command buffer recording. We will use this command buffer exactly once,
    // so we want to let Vulkan know that
    VkCommandBufferBeginInfo cmdBeginInfo {};
    cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBeginInfo.pNext = nullptr;
    cmdBeginInfo.pInheritanceInfo = nullptr;
    cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

    // Make a clear-color from frame number. This will flash with a 120*pi frame period.
    VkClearValue clearValue;
    float flash = abs(sin(frameNumber / 120.f));
    clearValue.color = {{0.0f, 0.0f, flash, 1.0f}};

    // Start the main renderpass.
    // We will use the clear color from above, and the framebuffer of the index the swapchain gave us.
    VkRenderPassBeginInfo renderPassBeginInfo {};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.pNext = nullptr;
    renderPassBeginInfo.renderPass = renderPass;
    renderPassBeginInfo.renderArea.offset.x = 0;
    renderPassBeginInfo.renderArea.offset.y = 0;
    renderPassBeginInfo.renderArea.extent = windowExtent;
    renderPassBeginInfo.framebuffer = framebuffers[swapchainImageIndex];
    renderPassBeginInfo.clearValueCount = 1;
    renderPassBeginInfo.pClearValues = &clearValue;
    vkCmdBeginRenderPass(cmd, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    // -- DRAW HERE --

    if (selectedShader == 0) {
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, trianglePipeline);
    } else {
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, redTrianglePipeline);
    }
    vkCmdDraw(cmd, 3, 1, 0, 0);

    //game.draw();


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
    submitInfo.pWaitSemaphores = &presentSemaphore;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &renderSemaphore;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;

    // Submit command buffer to the queue and execute it.
    // renderFence will now block until the graphic commands finish execution
    VK_CHECK(vkQueueSubmit(graphicsQueue, 1, &submitInfo, renderFence));

    // This will put the image we just rendered into the visible window.
    // We want to wait on the renderSemaphore for that,
    // as it's necessary that drawing commands have finished before the image is displayed to the user.
    VkPresentInfoKHR presentInfo {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;
    presentInfo.pSwapchains = &swapchain;
    presentInfo.swapchainCount = 1;
    presentInfo.pWaitSemaphores = &renderSemaphore;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pImageIndices = &swapchainImageIndex;

    VK_CHECK(vkQueuePresentKHR(graphicsQueue, &presentInfo));

    // Increase the number of frames drawn
    frameNumber++;
}

void Engine::run() {

    Timer timer;

    game.load();
    while (game.isRunning) {
        uint32_t dt = timer.computeDeltaTime();
        window.updateFpsCounter(dt);

        processInputs();
        game.update(dt);
        draw();

        // Frame delay is managed by the renderer,
        // which synchronizes with the monitor framerate.
    }
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
    SDL_Vulkan_CreateSurface(window.get(), instance, &surface);

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
}

void Engine::cleanupVulkan() {
    vkDestroyCommandPool(device, commandPool, nullptr);
    vkDestroySwapchainKHR(device, swapchain, nullptr);
    vkDestroyRenderPass(device, renderPass, nullptr);

    for (int i = 0; i < swapchainImages.size(); ++i) {
        vkDestroyFramebuffer(device, framebuffers[i], nullptr);
        vkDestroyImageView(device, swapchainImageViews[i], nullptr);
    }

    vkDestroyDevice(device, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
#ifdef _DEBUG
    vkb::destroy_debug_utils_messenger(instance, debugMessenger);
#endif
    vkDestroyInstance(instance, nullptr);
}

void Engine::initSwapchain() {
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
}

void Engine::initCommands() {
    // Command pool
    auto commandPoolInfo = vk::commandPoolCreateInfo(graphicsQueueFamily,
                                                     VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    VK_CHECK(vkCreateCommandPool(device, &commandPoolInfo, nullptr, &commandPool));

    // Command buffer
    auto cmdAllocInfo = vk::commandBufferAllocateInfo(commandPool, 1);
    VK_CHECK(vkAllocateCommandBuffers(device, &cmdAllocInfo, &mainCommandBuffer));
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

    // -- SUBPASS --
    VkSubpassDescription subpass {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    // -- RENDERPASS --
    VkRenderPassCreateInfo vkRenderPassCreateInfo {};
    vkRenderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    // Connect the color attachment to the info
    vkRenderPassCreateInfo.attachmentCount = 1;
    vkRenderPassCreateInfo.pAttachments = &colorAttachment;
    // Connect the subpass to the info
    vkRenderPassCreateInfo.subpassCount = 1;
    vkRenderPassCreateInfo.pSubpasses = &subpass;

    VK_CHECK(vkCreateRenderPass(device, &vkRenderPassCreateInfo, nullptr, &renderPass));
}

void Engine::initFramebuffers() {
    // Create the framebuffers for the swapchain images.
    // This will connect the render-pass to the images for rendering
    VkFramebufferCreateInfo framebufferCreateInfo = {};
    framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferCreateInfo.pNext = nullptr;
    framebufferCreateInfo.renderPass = renderPass;
    framebufferCreateInfo.attachmentCount = 1;
    framebufferCreateInfo.width = windowExtent.width;
    framebufferCreateInfo.height = windowExtent.height;
    framebufferCreateInfo.layers = 1;

    // Grab how many images we have in the swapchain
    const uint32_t swapchainImageCount = swapchainImages.size();
    framebuffers = std::vector<VkFramebuffer>(swapchainImageCount);

    // Create framebuffers for each of the swapchain image views
    for (size_t i = 0; i < swapchainImageCount; ++i) {
        framebufferCreateInfo.pAttachments = &swapchainImageViews[i];
        VK_CHECK(vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr, &framebuffers[i]));
    }
}

void Engine::initSyncStructures() {
    VkFenceCreateInfo fenceCreateInfo {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.pNext = nullptr;
    // We want to create the fence with the Create Signaled flag, so we can
    // wait on it before using it on a GPU command (for the first frame)
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    VK_CHECK(vkCreateFence(device, &fenceCreateInfo, nullptr, &renderFence));

    // For the semaphores we don't need any flags
    VkSemaphoreCreateInfo semaphoreCreateInfo = {};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreCreateInfo.pNext = nullptr;
    semaphoreCreateInfo.flags = 0;
    VK_CHECK(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &presentSemaphore));
    VK_CHECK(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &renderSemaphore));
}

bool Engine::loadShaderModule(const char* path, VkShaderModule* outShaderModule) {
    // Open the file stream in binary mode and put cursor at end
    std::ifstream file {path, std::ios::ate | std::ios::binary};
    if (!file.is_open()) {
        return false;
    }
    // Find what the size of the file is by looking up the location of the cursor, which is end of file.
    size_t fileSize = (size_t) file.tellg();
    // Spirv expects the buffer to be on uint32, so make sure to reserve an int vector big enough for the entire file.
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

void Engine::initPipelines() {
    // -- SHADER MODULES --
    VkShaderModule triangleFragShader;
    if (!loadShaderModule("../../shaders/triangle.frag.spv", &triangleFragShader)) {
        LOG(LogLevel::Error) << "Error when building the triangle fragment shader module";
    } else {
        LOG(LogLevel::Info) << "Triangle fragment shader successfully loaded";
    }

    VkShaderModule triangleVertexShader;
    if (!loadShaderModule("../../shaders/triangle.vert.spv", &triangleVertexShader)) {
        LOG(LogLevel::Error) << "Error when building the triangle vertex shader module";
    } else {
        LOG(LogLevel::Info) << "Triangle vertex shader successfully loaded";
    }

    VkShaderModule redTriangleFragShader;
    if (!loadShaderModule("../../shaders/redTriangle.frag.spv", &redTriangleFragShader)) {
        LOG(LogLevel::Error) << "Error when building the red triangle fragment shader module";
    } else {
        LOG(LogLevel::Info) << "Red triangle fragment shader successfully loaded";
    }

    VkShaderModule redTriangleVertexShader;
    if (!loadShaderModule("../../shaders/redTriangle.vert.spv", &redTriangleVertexShader)) {
        LOG(LogLevel::Error) << "Error when building the red triangle vertex shader module";
    } else {
        LOG(LogLevel::Info) << "Red triangle vertex shader successfully loaded";
    }

    // -- LAYOUT --
    // The pipeline layout that controls the inputs/outputs of the shader
    // We are not using descriptor sets or other systems yet, so no need to use anything other than empty default
    VkPipelineLayoutCreateInfo pipeline_layout_info = vk::pipelineLayoutCreateInfo();
    VK_CHECK(vkCreatePipelineLayout(device, &pipeline_layout_info, nullptr, &trianglePipelineLayout));

    // -- BUILD PIPELINE --
    VkPipelineBuilder pipelineBuilder;

    //	Build both the vertex and fragment stages. This lets the pipeline know the shader modules per stage.
    pipelineBuilder.shaderStages.push_back(
            vk::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, triangleVertexShader)
    );
    pipelineBuilder.shaderStages.push_back(
            vk::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, triangleFragShader)
    );

    // Vertex input controls how to read vertices from vertex buffers.
    pipelineBuilder.vertexInputInfo = vk::vertexInputStateCreateInfo();

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

    // Layout
    pipelineBuilder.pipelineLayout = trianglePipelineLayout;

    // Build
    trianglePipeline = pipelineBuilder.buildPipeline(device, renderPass);

    // Overwrite pipeline to use different shaders
    pipelineBuilder.shaderStages.clear();
    pipelineBuilder.shaderStages.push_back(
            vk::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, redTriangleVertexShader));
    pipelineBuilder.shaderStages.push_back(
            vk::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, redTriangleFragShader));
    redTrianglePipeline = pipelineBuilder.buildPipeline(device, renderPass);
}

void engine::Engine::processInputs() {
    inputSystem.preUpdate();
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        game.isRunning = inputSystem.processEvent(event);
    }
    inputSystem.update();

    InputState state = inputSystem.getInputState();
    if (state.keyboard.getKeyState(SDL_SCANCODE_SPACE) == ButtonState::Pressed) {
        selectedShader = ++selectedShader % 2;
    }
}
