//
// Created by gaetz on 08/06/2021.
//

#ifndef VK_TYPES_H
#define VK_TYPES_H

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include "../math/Types.h"

using engine::math::Vec4;
using engine::math::Mat4;

namespace engine::vk {

struct AllocatedBuffer {
    VkBuffer buffer;
    VmaAllocation allocation;
};

struct AllocatedImage {
    VkImage image;
    VmaAllocation allocation;
};

struct MeshPushConstants {
    Vec4 data;
    Mat4 renderMatrix;
};

struct FrameData {
    VkSemaphore presentSemaphore, renderSemaphore;
    VkFence renderFence;

    VkCommandPool commandPool;
    VkCommandBuffer mainCommandBuffer;
};

}

#endif //VK_TYPES_H
