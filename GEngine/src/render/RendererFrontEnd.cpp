//
// Created by gaetz on 08/10/2021.
//

#include "RendererFrontEnd.h"
#include "../Locator.h"
#include "vk/RendererBackEndVulkan.h"

using engine::render::RendererFrontEnd;

bool RendererFrontEnd::init(const string &appName) {
    // TODO Make the renderer type configurable
    backEnd = static_cast<RendererBackEndVulkan*>(Locator::memory().allocate(sizeof(RendererBackEndVulkan), engine::MemoryTag::Renderer));

    if (!backEnd->init(appName)) {
        LOG(engine::LogLevel::Fatal) << "Renderer backend failed to initialize, shutting down.";
        return false;
    }
    return true;
}

void RendererFrontEnd::close() {
    backEnd->close();
    // TODO Make the renderer type configurable
    Locator::memory().free(backEnd, sizeof(RendererBackEndVulkan), engine::MemoryTag::Renderer);
}

bool RendererFrontEnd::beginFrame(u32 dt) {
    return backEnd->beginFrame(dt);
}

bool RendererFrontEnd::endFrame(u32 dt) {
    bool result = backEnd->endFrame(dt);
    backEnd->incrementFrame();
    return result;
}

bool RendererFrontEnd::drawFrame(const RenderPacket &packet) {
    // If begin frame return successfully, mid-frame operations can continue
    if (backEnd->beginFrame(packet.dt)) {

        bool frameResult = backEnd->endFrame(packet.dt);
        if (!frameResult) {
            LOG(LogLevel::Error) << "Renderer end frame failed. Application shutting down.";
            return false;
        }
    }
    return true;
}

void RendererFrontEnd::onRezise(u16 width, u16 height) {

}

