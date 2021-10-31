//
// Created by gaetz on 31/10/2021.
//

#include "Buffer.h"

using engine::render::vk::Buffer;

engine::render::vk::Buffer::~Buffer() {
    if (state != BufferState::Destroyed) destroy();
}

void Buffer::init(Context& contextP, VkBufferUsageFlags usageP, VmaMemoryUsage memoryUsageP, u64 sizeP) {
    context = &contextP;
    size = sizeP;
    usage = usageP;
    memoryUsage = memoryUsageP;

    VkBufferCreateInfo bufferInfo {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext = nullptr;
    bufferInfo.size = size;
    bufferInfo.usage = usage;

    VmaAllocationCreateInfo vmaAllocInfo {};
    vmaAllocInfo.usage = memoryUsage;

    // Allocate memory
    VK_CHECK(vmaCreateBuffer(context->allocator,
                             &bufferInfo,
                             &vmaAllocInfo,
                             &handle,
                             &allocation,
                             nullptr));
    state = BufferState::Allocated;
}

void engine::render::vk::Buffer::destroy() {
    if (state != BufferState::Empty) {
        vmaDestroyBuffer(context->allocator, handle, allocation);
    }
    size = 0;
    usage = 0;
    state = BufferState::Destroyed;
}

bool Buffer::resize(u64 newSize, VkQueue queue, VkCommandPool cmdPool) {
    VkBuffer newBufferHandle;
    VmaAllocation newBufferAllocation;

    VkBufferCreateInfo bufferInfo {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext = nullptr;
    bufferInfo.size = newSize;
    bufferInfo.usage = usage;

    VmaAllocationCreateInfo vmaAllocInfo {};
    vmaAllocInfo.usage = memoryUsage;

    // Allocate memory
    VK_CHECK(vmaCreateBuffer(context->allocator,
                             &bufferInfo,
                             &vmaAllocInfo,
                             &newBufferHandle,
                             &newBufferAllocation,
                             nullptr));

    // Copy old data to new data then destroy old data
    copyTo(newBufferHandle, 0, 0, size, cmdPool, 0, queue);
    vkDeviceWaitIdle(context->device);
    vmaDestroyBuffer(context->allocator, handle, allocation);

    // Set new data back in buffer
    size = newSize;
    handle = newBufferHandle;
    allocation = newBufferAllocation;

    return true;
}

void Buffer::bind() {
    vmaBindBufferMemory(context->allocator, allocation, handle);
}

/*
void Buffer::bind(u64 offset) {
    vmaBindBufferMemory2(context.allocator, allocation, offset, handle, nullptr);
}
 */

void* Buffer::lockMemory() {
    state = BufferState::Locked;
    void* data;
    vmaMapMemory(context->allocator, allocation, &data);
    return data;
}

/*
void* Buffer::lockMemory(u64 offset, u64 size, u32 flags) {
    void* data;
    VK_CHECK(vkMapMemory(context.device, allocation->GetMemory(), offset, size, flags, &data));
    return data;
}
*/

void engine::render::vk::Buffer::loadData(const void* data, u64 size) {
    void* dataPtr = lockMemory();
    memcpy(dataPtr, data, size);
    unlockMemory();
}

void Buffer::unlockMemory() {
    vmaUnmapMemory(context->allocator, allocation);
    state = BufferState::Allocated;
}

void Buffer::copyTo(VkBuffer dst, u64 srcOffset, u64 dstOffset, u64 sizeP, VkCommandPool cmdPool, VkFence fence,
                    VkQueue queue) {
    vkQueueWaitIdle(queue);
    CommandBuffer cmd;
    cmd.singleUseBegin(*context, cmdPool);

    // Prepare copy region then submit copy command
    VkBufferCopy copyRegion;
    copyRegion.srcOffset = srcOffset;
    copyRegion.dstOffset = dstOffset;
    copyRegion.size = sizeP;
    vkCmdCopyBuffer(cmd.handle, handle, dst, 1, &copyRegion);

    cmd.singleUseEnd(*context, cmdPool, queue);
}




