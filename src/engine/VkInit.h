//
// Created by gaetz on 08/06/2021.
//

#ifndef VKINIT_H
#define VKINIT_H

#include "Types.h"

namespace vkinit {

    // Create a command pool for commands submitted to the graphics queue.
    VkCommandPoolCreateInfo commandPoolCreateInfo(uint32_t queueFamilyIndex,
                                                  VkCommandPoolCreateFlags flags = 0);

    // Allocate the default command buffer that we will use for rendering
    VkCommandBufferAllocateInfo commandBufferAllocateInfo(VkCommandPool pool,
                                                          uint32_t count = 1,
                                                          VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
}



#endif //VKINIT_H
