//
// Created by gaetz on 10/10/2021.
//
#include "VkContext.h"
#include <vk_mem_alloc.h>

#include "VkInit.h"
#include "../../externals/vkbootstrap/VkBootstrap.h"
#include "../../Locator.h"

using engine::LogLevel;

void engine::render::vk::VkContext::init(const string& appName) {
    // -- INSTANCE --
    vkb::InstanceBuilder builder;

#ifdef _DEBUG
    // Make vulkan instance with basic debug features
    auto instanceResult = builder.set_app_name(appName.c_str())
            .request_validation_layers(true)
            .require_api_version(1, 1, 0)
            .set_debug_callback (
                    [] (VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                        VkDebugUtilsMessageTypeFlagsEXT messageType,
                        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                        void *pUserData)
                            -> VkBool32 {
                        auto severity = vkb::to_string_message_severity (messageSeverity);
                        auto type = vkb::to_string_message_type (messageType);
                        LOG(vkSeverityToLogLevel(messageSeverity)) << "VK[" << type << "] " << pCallbackData->pMessage;
                        return VK_FALSE;
                    }
            )
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
    // Setup the surface
    Locator::platform().rendererSetupVulkanSurface(instance, &surface);

    // Use VkBootstrap to select a GPU.
    // We want a GPU that can write to the SDL surface and supports Vulkan 1.1
    vkb::PhysicalDeviceSelector selector {vkbInstance};
    vkb::PhysicalDevice physicalDevice = selector
            .set_minimum_version(1, 1)
            .set_surface(surface)
            .prefer_gpu_device_type(vkb::PreferredDeviceType::discrete)
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

void engine::render::vk::VkContext::close() {
    mainDeletionQueue.flush();
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyDevice(device, nullptr);
#ifdef _DEBUG
    vkb::destroy_debug_utils_messenger(instance, debugMessenger);
#endif
    vkDestroyInstance(instance, nullptr);
}

LogLevel engine::render::vk::vkSeverityToLogLevel(VkDebugUtilsMessageSeverityFlagBitsEXT severity) {
    switch (severity) {
        case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            return LogLevel::Trace;
        case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            return LogLevel::Error;
        case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            return LogLevel::Warning;
        case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            return LogLevel::Info;
        default:
            return LogLevel::Trace;
    }
}