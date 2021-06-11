//
// Created by gaetz on 08/06/2021.
//

#include "VkInit.h"

VkCommandPoolCreateInfo vkinit::commandPoolCreateInfo(uint32_t queueFamilyIndex,
                                                      VkCommandPoolCreateFlags flags) {
    VkCommandPoolCreateInfo commandPoolInfo{};
    commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolInfo.pNext = nullptr;
    commandPoolInfo.queueFamilyIndex = queueFamilyIndex;
    commandPoolInfo.flags = flags;

    return commandPoolInfo;
}

VkCommandBufferAllocateInfo vkinit::commandBufferAllocateInfo(VkCommandPool pool,
                                                              uint32_t count,
                                                              VkCommandBufferLevel level) {
    VkCommandBufferAllocateInfo cmdAllocInfo{};
    cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdAllocInfo.pNext = nullptr;
    cmdAllocInfo.commandPool = pool;
    cmdAllocInfo.commandBufferCount = count;
    cmdAllocInfo.level = level;
    return cmdAllocInfo;
}


