//
// Created by gaetz on 23/10/2021.
//

#ifndef RENDER_VK_COMMANDBUFFER_H
#define RENDER_VK_COMMANDBUFFER_H

#include <vulkan/vulkan_core.h>

namespace engine::render::vk {

    class Context;

    enum class CommandBufferState {
        Ready,
        Recording,
        InRenderPass,
        RecordingEnded,
        Submitted,
        NotAllocated
    };

    class CommandBuffer {
    public:
        VkCommandBuffer handle { nullptr };
        CommandBufferState state { CommandBufferState::NotAllocated };

        void allocate(Context& context, VkCommandPool pool, bool isPrimary, bool shouldFree);
        void free(const Context& context, VkCommandPool pool);

        /***
         *
         * @param isSingleUse
         * @param isRenderPassContinue
         * @param isSimultaneousUse
         */
        void begin(bool isSingleUse, bool isRenderPassContinue, bool isSimultaneousUse);
        void end();

        void updateSubmitted();
        void reset();
        void singleUseBegin(Context& context, VkCommandPool pool);
        void singleUseEnd(const Context& context, VkCommandPool pool, VkQueue queue);
        void singleUseEnd(const Context& context, VkCommandPool pool, VkQueue queue, VkFence fence);
    };
}

#endif //RENDER_VK_COMMANDBUFFER_H
