//
// Created by gaetz on 23/10/2021.
//

#ifndef RENDER_VK_RENDERPASS_H
#define RENDER_VK_RENDERPASS_H

#include "../../Defines.h"
#include "Swapchain.h"
#include "../../math/Types.h"
#include "CommandBuffer.h"
#include "Context.h"

using engine::math::Vec4;
using engine::render::vk::Context;
using engine::render::vk::Swapchain;

namespace engine::render::vk {
    class Framebuffer;

    enum class RenderPassState {
        Ready,
        Recording,
        InRenderPass,
        RecordingEnded,
        Submitted,
        NotAllocated
    };

    class Renderpass {
    public:
        RenderPassState state { RenderPassState::NotAllocated };
        VkRenderPass handle { VK_NULL_HANDLE };

        explicit Renderpass(Context& contextP, const Vec4& renderZoneP, const Vec4& clearColorP, f32 depthP, u32 stencilP);
        void init(const Swapchain& swapchain);
        void destroy();
        void begin(CommandBuffer& commandBuffer, const Framebuffer& framebuffer);
        void end(CommandBuffer& commandBuffer);

    private:
        Context& context;
        Vec4 renderZone;
        Vec4 clearColor;
        f32 depth;
        u32 stencil;

    };
}


#endif //RENDER_VK_RENDERPASS_H
