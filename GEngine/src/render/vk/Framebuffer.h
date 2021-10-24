//
// Created by gaetz on 23/10/2021.
//

#ifndef RENDER_VK_FRAMEBUFFER_H
#define RENDER_VK_FRAMEBUFFER_H

#include <array>
#include <vulkan/vulkan_core.h>
#include "../../Defines.h"

using std::array;

namespace engine::render::vk {
    class Context;
    class Renderpass;
    class Swapchain;

    class Framebuffer {
    public:
        explicit Framebuffer(Context& contextP, Renderpass& renderpassP);
        void init(VkImageView imageView, VkImageView depthImageView);
        void destroy();

        VkFramebuffer handle;
        u32 getAttachmentCount() { return attachments.size(); }
        VkImageView* getAttachments() { return attachments.data(); }

    private:
        Context& context;
        Renderpass& renderpass;
        array<VkImageView, 2> attachments;

    };

}

#endif //RENDER_VK_FRAMEBUFFER_H
