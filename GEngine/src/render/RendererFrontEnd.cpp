//
// Created by gaetz on 08/10/2021.
//

#include "RendererFrontEnd.h"
#include "vk/RendererBackEndVulkan.h"
#include "../math/Transformations.h"
#include "../math/Functions.h"
#include "../../../externals/imgui/imgui.h"


using engine::render::RendererFrontEnd;

bool RendererFrontEnd::init(const string &appName, u16 widthP, u16 heightP) {
    // TODO Make the renderer type configurable
    //backEnd = static_cast<RendererBackEndVulkan*>(Locator::memory().allocate(sizeof(RendererBackEndVulkan), MemoryTag::Renderer));
    backEnd = static_cast<RendererBackEnd *>(new vk::RendererBackEndVulkan());
    width = widthP;
    height = heightP;

    if (!backEnd->init(appName, widthP, heightP)) {
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

        Vec3 camPos { 0.f, -6.f, -10.f };
        Mat4 view = math::translate(Mat4 { 1.f }, camPos);
        Mat4 projection = math::perspective(math::toRad(70.f),
                                            static_cast<float>(width) /
                                            static_cast<float>(height),
                                            0.1f, 200.f);

        backEnd->updateGlobalState(projection, view);
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

void engine::render::RendererFrontEnd::createMaterial(const string& name, const string& shaderName, const string& textureName) {
    backEnd->createMaterial(name, shaderName, textureName);
}

