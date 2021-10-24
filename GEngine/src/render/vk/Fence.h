//
// Created by gaetz on 24/10/2021.
//

#ifndef GAEMI01_FENCE_H
#define GAEMI01_FENCE_H


#include <vulkan/vulkan_core.h>
#include "../../Defines.h"

namespace engine::render::vk {
    class Context;

    class Fence {
    public:
        VkFence handle { nullptr };
        bool signaled { false };

        void init(Context& context, bool isCreatedSignaled);
        void destroy(const Context& context);
        bool wait(const Context& context, u64 timeout);
        void reset(const Context& context);
    };
}




#endif //GAEMI01_FENCE_H
