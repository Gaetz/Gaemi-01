//
// Created by gaetz on 23/10/2021.
//

#include "CommandBuffer.h"
#include "Init.h"
#include "Context.h"

using engine::render::vk::CommandBuffer;
using engine::render::vk::Context;

void CommandBuffer::allocate(Context &context, VkCommandPool pool, bool isPrimary, bool shouldFree) {
    auto level = isPrimary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    auto cmdAllocInfo = render::vk::commandBufferAllocateInfo(pool, 1, level);

    state = CommandBufferState::NotAllocated;
    VK_CHECK(vkAllocateCommandBuffers(context.device, &cmdAllocInfo, &handle));
    state = CommandBufferState::Ready;
    if (!shouldFree) return;
    context.mainDeletionQueue.pushFunction([=]() {
        free(context, pool);
    });
}

void CommandBuffer::free(const Context &context, VkCommandPool pool) {
    vkFreeCommandBuffers(context.device, pool, 1, &handle);
    handle = 0;
    state = CommandBufferState::NotAllocated;
}

void CommandBuffer::begin(bool isSingleUse, bool isRenderPassContinue, bool isSimultaneousUse) {
    u32 flags;
    if (isSingleUse) flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    if (isRenderPassContinue) flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    if (isSimultaneousUse) flags |= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

    VkCommandBufferBeginInfo cmdBeginInfo = render::vk::commandBufferBeginInfo(
            static_cast<VkCommandBufferUsageFlagBits>(flags)
    );
    VK_CHECK(vkBeginCommandBuffer(handle, &cmdBeginInfo));
    state = CommandBufferState::Recording;
}

void CommandBuffer::end() {
    VK_CHECK(vkEndCommandBuffer(handle));
    state = CommandBufferState::RecordingEnded;
}

void CommandBuffer::updateSubmitted() {
    state = CommandBufferState::Submitted;
}

void CommandBuffer::reset() {
    VK_CHECK(vkResetCommandBuffer(handle, 0));
    state = CommandBufferState::Ready;
}

void CommandBuffer::singleUseBegin(Context &context, VkCommandPool pool) {
    allocate(context, pool, true, false);
    begin(true, false, false);
}

void CommandBuffer::singleUseEnd(const engine::render::vk::Context &context, VkCommandPool pool,
                                 VkQueue queue) {
    end();

    VkSubmitInfo submitInfo {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;
    submitInfo.waitSemaphoreCount = 0;
    submitInfo.pWaitSemaphores = nullptr;
    submitInfo.pWaitDstStageMask = nullptr;
    submitInfo.signalSemaphoreCount = 0;
    submitInfo.pSignalSemaphores = nullptr;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &handle;

    // Submit command buffer to the queue and execute it.
    VK_CHECK(vkQueueSubmit(context.graphicsQueue, 1, &submitInfo, 0));
    VK_CHECK(vkQueueWaitIdle(queue));

    free(context, pool);
}

void CommandBuffer::singleUseEnd(const engine::render::vk::Context &context, VkCommandPool pool,
                                                     VkQueue queue, VkFence fence) {
    end();

    VkSubmitInfo submitInfo {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;
    submitInfo.waitSemaphoreCount = 0;
    submitInfo.pWaitSemaphores = nullptr;
    submitInfo.pWaitDstStageMask = nullptr;
    submitInfo.signalSemaphoreCount = 0;
    submitInfo.pSignalSemaphores = nullptr;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &handle;

    // Submit command buffer to the queue and execute it.
    // uploadFence will now block until the graphic commands finish execution
    VK_CHECK(vkQueueSubmit(context.graphicsQueue, 1, &submitInfo, fence));
    VK_CHECK(vkWaitForFences(context.device, 1, &fence, true, 9999999999));
    VK_CHECK(vkResetFences(context.device, 1, &fence));

    free(context, pool);
}




