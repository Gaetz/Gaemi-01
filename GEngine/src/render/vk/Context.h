//
// Created by gaetz on 10/10/2021.
//

#ifndef RENDER_VK_CONTEXT_H
#define RENDER_VK_CONTEXT_H

#include <string>
using std::string;

#include "Types.h"
#include "../../Log.h"

namespace engine::render::vk {
    class Context {
    public:
        void init(const string& appName);
        void close();
        void waitIdle() const;

        VkExtent2D windowExtent;
        VkInstance instance;
        VkDebugUtilsMessengerEXT debugMessenger;
        VkPhysicalDevice chosenGPU;
        VkDevice device;
        VkPhysicalDeviceProperties gpuProperties;
        VmaAllocator allocator;
        VkSurfaceKHR surface;

        VkQueue graphicsQueue;
        uint32_t graphicsQueueFamily;

        DeletionQueue mainDeletionQueue;
    };

    engine::LogLevel vkSeverityToLogLevel(VkDebugUtilsMessageSeverityFlagBitsEXT severity);
}

#endif //RENDER_VK_CONTEXT_H
