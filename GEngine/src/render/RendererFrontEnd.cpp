//
// Created by gaetz on 08/10/2021.
//

#include "RendererFrontEnd.h"
#include "vk/RendererBackEndVulkan.h"
#include "../Locator.h"


using engine::render::RendererFrontEnd;
using engine::mem::MemoryTag;

bool RendererFrontEnd::init(const string &appName, u16 width, u16 height) {
    // TODO Make the renderer type configurable
    //backEnd = static_cast<RendererBackEndVulkan*>(Locator::memory().allocate(sizeof(RendererBackEndVulkan), MemoryTag::Renderer));
    backEnd = static_cast<RendererBackEnd *>(new vk::RendererBackEndVulkan());

    if (!backEnd->init(appName, width, height)) {
        LOG(engine::LogLevel::Fatal) << "Renderer backend failed to initialize, shutting down.";
        return false;
    }
    return true;
}

void RendererFrontEnd::close() {
    backEnd->close();
    // TODO Make the renderer type configurable
    //Locator::memory().free(backEnd, sizeof(RendererBackEndVulkan), MemoryTag::Renderer);
    delete backEnd;
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

        backEnd->draw();

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

engine::render::vk::Texture RendererFrontEnd::loadTexture(const string &path) {
    return backEnd->loadTexture(path);
}

void RendererFrontEnd::upload(engine::render::vk::Mesh &mesh) {
    backEnd->uploadMesh(mesh);
}

void RendererFrontEnd::addToScene(engine::render::vk::GameObject &gameObject) {
    backEnd->addToScene(gameObject);
}

void engine::render::RendererFrontEnd::waitIdle() {
    backEnd->waitIdle();
}

void engine::render::RendererFrontEnd::createMaterial(const string& name) {
    backEnd->createMaterial(name);
}

