#include "Engine.h"

#ifdef __linux__
    #include <SDL2/SDL.h>
#elif _WIN32
	#include <SDL.h>
#endif

#include "Types.h"
#include "VkInit.h"
#include "Timer.h"
#include "Log.h"
#include "../../externals/vkbootstrap/VkBootstrap.h"

#ifdef _DEBUG
#define VK_CHECK(x)                                                 \
	do                                                              \
	{                                                               \
		VkResult err = x;                                           \
		if (err)                                                    \
		{                                                           \
			LOG(Error) << "Detected Vulkan error: " << err;         \
			abort();                                                \
		}                                                           \
	} while (0)
#endif


void Engine::init() {
    SDL_Init(SDL_INIT_VIDEO);
    window.init(windowExtent.width, windowExtent.height, false);
    initVulkan();
    initSwapchain();
    initCommands();
    initDefaultRenderpass();
    initFramebuffers();
    initSyncStructures();
    isInitialized = true;
}

void Engine::cleanup() {
    if (isInitialized) {
        vulkanCleanup();
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
    VkCommandBufferBeginInfo cmdBeginInfo{};
    cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBeginInfo.pNext = nullptr;
    cmdBeginInfo.pInheritanceInfo = nullptr;
    cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

    // Make a clear-color from frame number. This will flash with a 120*pi frame period.
    VkClearValue clearValue;
    float flash = abs(sin(frameNumber / 120.f));
    clearValue.color = { { 0.0f, 0.0f, flash, 1.0f } };

    // Start the main renderpass.
    // We will use the clear color from above, and the framebuffer of the index the swapchain gave us.
    VkRenderPassBeginInfo renderPassBeginInfo{};
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


    game.draw();


    // Finalize the render pass
    vkCmdEndRenderPass(cmd);
    // Finalize the command buffer (we can no longer add commands, but it can now be executed)
    VK_CHECK(vkEndCommandBuffer(cmd));

    // Prepare the submission to the queue.
    // We want to wait on the presentSemaphore, as that semaphore is signaled when the swapchain is ready.
    // We will signal the renderSemaphore, to signal that rendering has finished.
    VkSubmitInfo submitInfo{};
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
    VkPresentInfoKHR presentInfo{};
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
    while (game.isRunning)
    {
        uint32_t dt = timer.computeDeltaTime();
        window.updateFpsCounter(dt);

        game.handleInputs();
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
    vkb::PhysicalDeviceSelector selector{ vkbInstance };
    vkb::PhysicalDevice physicalDevice = selector
            .set_minimum_version(1, 1)
            .set_surface(surface)
            .select()
            .value();

    // Create the final Vulkan device
    vkb::DeviceBuilder deviceBuilder{ physicalDevice };
    vkb::Device vkbDevice = deviceBuilder.build().value();

    // Get the VkDevice handle used in the rest of a Vulkan application
    device = vkbDevice.device;
    chosenGPU = physicalDevice.physical_device;

    // Get graphics queue
    graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
    graphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();
}

void Engine::vulkanCleanup() {
    vkDestroyCommandPool(device, commandPool, nullptr);
    vkDestroySwapchainKHR(device,swapchain, nullptr);
    vkDestroyRenderPass(device, renderPass, nullptr);

    for(int i = 0; i < swapchainImages.size(); ++i) {
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
    vkb::SwapchainBuilder swapchainBuilder { chosenGPU, device, surface };
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
    auto commandPoolInfo = vkinit::commandPoolCreateInfo(graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    VK_CHECK(vkCreateCommandPool(device, &commandPoolInfo, nullptr, &commandPool));

    // Command buffer
    auto cmdAllocInfo = vkinit::commandBufferAllocateInfo(commandPool, 1);
    VK_CHECK(vkAllocateCommandBuffers(device, &cmdAllocInfo, &mainCommandBuffer));
}

void Engine::initDefaultRenderpass() {
    // -- COLOR ATTACHMENT --
    // The renderpass will use this color attachment.
    VkAttachmentDescription colorAttachment{};
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
    VkAttachmentReference colorAttachmentRef{};
    // Attachment number will index into the pAttachments array in the parent renderpass itself
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // -- SUBPASS --
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    // -- RENDERPASS --
    VkRenderPassCreateInfo vkRenderPassCreateInfo{};
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
    VkFenceCreateInfo fenceCreateInfo{};
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
