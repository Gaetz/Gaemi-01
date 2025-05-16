//
// Created by gaetz on 31/10/2021.
//

#ifndef RENDER_VK_BUFFER_H
#define RENDER_VK_BUFFER_H

#include <vulkan/vulkan_core.h>
#include <vma/vk_mem_alloc.h>
#include "../../Defines.h"
#include "Context.h"

namespace engine::render::vk {

    enum class BufferState {
        Empty,
        Allocated,
        Locked,
        Destroyed
    };

    class Buffer {
    public:
        ~Buffer();

        void init(Context& context, VkBufferUsageFlags usageP, VmaMemoryUsage memoryUsageP, u64 size);
        void destroy();
        bool resize(u64 newSize, VkQueue queue, VkCommandPool cmdPool);

        void bind();
        //void bind(u64 offset); // Offset is managed by VMA
        void* lockMemory();
        //void* lockMemory(u64 offset, u64 size, u32 flags); // Offset is managed by VMA
        void unlockMemory();
        void loadData(const void* data, u64 size);
        void copyTo(VkBuffer dst, u64 srcOffset, u64 dstOffset, u64 size, VkCommandPool cmdPool, VkFence fence, VkQueue queue);

        u64 size { 0 };
        VkBuffer handle { nullptr };

    private:
        VkBufferUsageFlags usage {};
        VmaMemoryUsage memoryUsage {};
        VmaAllocation allocation;
        BufferState state { BufferState::Empty };
        Context* context { nullptr };
    };
}



#endif //RENDER_VK_BUFFER_H
