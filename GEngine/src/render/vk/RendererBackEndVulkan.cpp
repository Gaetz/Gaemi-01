//
// Created by gaetz on 08/10/2021.
//

#include "RendererBackEndVulkan.h"

using engine::render::RendererBackEndVulkan;

bool RendererBackEndVulkan::init(const string& appName) {
    return true;
}

void RendererBackEndVulkan::close() {

}

bool RendererBackEndVulkan::beginFrame(u32 dt) {
    return true;
}

bool RendererBackEndVulkan::endFrame(u32 dt) {
    return true;
}
