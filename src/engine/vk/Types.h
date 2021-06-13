//
// Created by gaetz on 08/06/2021.
//

#ifndef VK_TYPES_H
#define VK_TYPES_H

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

namespace engine::vk {

struct AllocatedBuffer {
    VkBuffer buffer;
    VmaAllocation allocation;
};

}

#endif //VK_TYPES_H
