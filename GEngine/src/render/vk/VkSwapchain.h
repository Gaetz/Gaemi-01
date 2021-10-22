//
// Created by gaetz on 10/10/2021.
//

#ifndef RENDER_VK_VKSWAPCHAIN_H
#define RENDER_VK_VKSWAPCHAIN_H

#include <vector>
using std::vector;

#include "Context.h"

namespace engine::render::vk {

    class VkSwapchain {
    public:
        explicit VkSwapchain(Context& contextP);
        void init();
        void reinit();
        void acquireNextImage(u64 timeout, VkSemaphore semaphore, VkFence fence, u32& outImageIndex);
        void present(VkSemaphore renderCompleteSemaphore, u32& imageIndex);

        VkSwapchainKHR handle;
        VkFormat imageFormat;
        vector<VkImage> images {};
        vector<VkImageView> imageViews {};
        u32 imageIndex { 0 };

        VkImageView depthImageView;
        render::vk::AllocatedImage depthImage;
        VkFormat depthFormat;

    private:
        void create();
        void destroy();

        std::function<void()> destroyHandle;
        std::function<void()> destroyDepthImageViews;
        std::function<void()> destroyDepthImages;

        Context& context;
        bool deletionQueueRegistered { false };
    };
}

#endif //RENDER_VK_VKSWAPCHAIN_H
