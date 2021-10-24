//
// Created by gaetz on 24/10/2021.
//

#include "Fence.h"
#include "Context.h"
#include "Init.h"

using engine::render::vk::Fence;
using engine::render::vk::Context;

void Fence::init(Context &context, bool isCreatedSignaled) {
    isSignaled = isCreatedSignaled;
    VkFenceCreateInfo fenceCreateInfo;
    if (isSignaled) {
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
    isSignaled = false;
}

bool Fence::wait(const Context &context, u64 timeout) {
    if (!isSignaled) {
        VkResult result = vkWaitForFences(context.device, 1, &handle, true, timeout);
        switch (result) {
            case VK_SUCCESS:
                isSignaled = true;
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
        // Already isSignaled, do not wait
        return true;
    }
    return false;
}

void Fence::reset(const Context &context) {
    if(isSignaled) {
        VK_CHECK(vkResetFences(context.device, 1, &handle));
        isSignaled = false;
    }
}
