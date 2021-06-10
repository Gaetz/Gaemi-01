#include "Engine.h"

#ifdef __linux__
    #include <SDL2/SDL.h>
#elif _WIN32
	#include <SDL.h>
#endif

#include "Types.h"
#include "VkInit.h"
#include "Timer.h"
#include "../../externals/vkbootstrap/VkBootstrap.h"

#ifdef DEBUG
#define VK_CHECK(x)                                                 \
	do                                                              \
	{                                                               \
		VkResult err = x;                                           \
		if (err)                                                    \
		{                                                           \
			LOG(Error) << "Detected Vulkan error: " << err          \
			abort();                                                \
		}                                                           \
	} while (0)
#endif


void Engine::init() {
    SDL_Init(SDL_INIT_VIDEO);
    initVulkan();
    //initSwapchain();
    isInitialized = window.init(windowExtent.width, windowExtent.height, false);
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
    game.draw();
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

        // Delay frame if game runs too fast
        timer.delayTime();
    }
}

void Engine::initVulkan() {
    // -- INSTANCE --
    vkb::InstanceBuilder builder;

#ifdef DEBUG
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
            .request_validation_layers(true)
            .require_api_version(1, 1, 0)
            .use_default_debug_messenger()
            .build();
    vkb::Instance vkbInstance = instanceResult.value();
    instance = vkbInstance.instance;
    debugMessenger = vkbInstance.debug_messenger;

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

void Engine::vulkanCleanup() {
    vkDestroySwapchainKHR(device,swapchain, nullptr);
    // Destroy swapchain resources
    for(int i = 0; i < swapchainImages.size(); ++i) {
        vkDestroyImageView(device, swapchainImageViews[i], nullptr);
    }

    vkDestroyDevice(device, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
#ifdef DEBUG
    vkb::destroy_debug_utils_messenger(instance, debugMessenger);
#endif
    vkDestroyInstance(instance, nullptr);
}
