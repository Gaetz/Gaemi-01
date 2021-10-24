//
// Created by gaetz on 24/10/2021.
//

#include "Fence.h"
#include "Context.h"
#include "Init.h"

using engine::render::vk::Fence;
using engine::render::vk::Context;

void Fence::init(Context &context, bool isCreatedSignaled) {
    signaled = isCreatedSignaled;
    VkFenceCreateInfo fenceCreateInfo;
    if (signaled) {
        fenceCreateInfo = render::vk::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
    } else {
        fenceCreateInfo = render::vk::fenceCreateInfo();
    }
    VK_CHECK(vkCreateFence(context.device, &fenceCreateInfo, nullptr, &handle));

    context.mainDeletionQueue.pushFunction([=]() {
        destroy(context);
    });
}

void Fence::destroy(const Context& context) {
    vkDestroyFence(context.device, handle, nullptr);
    signaled = false;
}

bool Fence::wait(const Context &context, u64 timeout) {
    if (!signaled) {
        VkResult result = vkWaitForFences(context.device, 1, &handle, true, timeout);
        switch (result) {
            case VK_SUCCESS:
                signaled = true;
                return true;
            case VK_TIMEOUT:
                LOG(LogLevel::Warning) << "Fence wait - Timed out";
                break;
            case VK_ERROR_DEVICE_LOST:
                LOG(LogLevel::Error) << "Fence wait - Device lost";
                break;
            case VK_ERROR_OUT_OF_HOST_MEMORY:
                LOG(LogLevel::Error) << "Fence wait - Out of host memory";
                break;
            case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                LOG(LogLevel::Error) << "Fence wait - Out of device memory";
                break;
            default:
                LOG(LogLevel::Error) << "Fence wait - Unknown error";
                break;
        }
    } else {
        // Already signaled, do not wait
        return true;
    }
    return false;
}

void Fence::reset(const Context &context) {
    if(signaled) {
        VK_CHECK(vkResetFences(context.device, 1, &handle));
        signaled = false;
    }
}
